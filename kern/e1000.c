#include <kern/e1000.h>
#include <kern/pmap.h>
#include <inc/string.h>
#include <inc/error.h>
#include <kern/picirq.h>

//#define LOG_REGS

static inline physaddr_t
kva2pa(void *kva)
{
	return page2pa(page_lookup(kern_pgdir, kva, NULL)) | PGOFF(kva);
}

volatile uintptr_t *e1000;
volatile struct e1000_tx_desc tx_queue[E1000_TX_Q_LEN];
volatile struct e1000_rx_desc rx_queue[E1000_RX_Q_LEN];
volatile e1000_tx_buffer_t tx_buffers[E1000_TX_Q_LEN];
volatile e1000_rx_buffer_t rx_buffers[E1000_RX_Q_LEN];

static void
e1000w(int index, int value)
{
#ifdef LOG_REGS
	cprintf("E1000: e1000w: idx 0x%08x val 0x%08x\n", index, value);
#endif
	e1000[index/4] = value;
}

static uint32_t
e1000r(int index)
{
	uint32_t value = e1000[index/4];
#ifdef LOG_REGS
	cprintf("E1000: e1000r: idx 0x%08x val 0x%08x\n", index, value);
#endif
	return value;
}

int
e1000_init(struct pci_func *pcif) 
{
	int i;

	// enable pci device and get BARs and sizes
	pci_func_enable(pcif);

	// map registers in memory
	e1000 = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);
	e1000r(E1000_STATUS);	//	for debug

	irq_setmask_8259A(irq_mask_8259A & ~(1 << pcif->irq_line));

	// init tx descriptors
	for (i = 0; i < E1000_TX_Q_LEN; i++) {
		tx_queue[i].buffer_addr = kva2pa((void*)&tx_buffers[i]);
		tx_queue[i].cmd |= 0x9;	// set RS & EOP bits
		tx_queue[i].status |= 0x1; // set DD bit
	}

	// init tx registers
	e1000w(E1000_TDBAL, kva2pa(&tx_queue));
	e1000w(E1000_TDBAH, 0);	// 0x0 for 32 bit system
	e1000w(E1000_TDLEN, E1000_TX_Q_LEN * sizeof(struct e1000_tx_desc));
	e1000w(E1000_TDH, 0);
	e1000w(E1000_TDT, 0);
	e1000w(E1000_TCTL, E1000_TCTL_EN | E1000_TCTL_PSP | (E1000_TCTL_CT & 0x10) | (E1000_TCTL_COLD & 0x40));
	uint32_t ipgt = 10;
	uint32_t ipgr2 = 6;
	uint32_t ipgr1 = (2*ipgr2)/3;
	e1000w(E1000_TIPG, ipgr2 << 20 | ipgr1 << 10 | ipgt << 0);

	// init rx descriptors
	for (i = 0; i < E1000_RX_Q_LEN; i++) {
		rx_queue[i].buffer_addr = kva2pa((void*)&rx_buffers[i]);
	}

	// init rx registers
	e1000w(E1000_RAL0, (E1000_MAC_ADDR3 << 24) | (E1000_MAC_ADDR2 << 16) | (E1000_MAC_ADDR1 << 8) | (E1000_MAC_ADDR0 << 0));
	e1000w(E1000_RAH0, /* set addr valid bit */  (0x1             << 31) | (E1000_MAC_ADDR5 << 8) | (E1000_MAC_ADDR4 << 0));
	for (i = 0; i < 0x200; i += 0x4) {
		e1000w(E1000_MTA + i, 0x0);
	}
	e1000w(E1000_IMC, ~0);
	e1000w(E1000_IMS, E1000_ICR_RXT0);
	e1000w(E1000_RDBAL, kva2pa(&rx_queue));
	e1000w(E1000_RDBAH, 0);	// 0x0 for 32 bit system
	e1000w(E1000_RDLEN, E1000_RX_Q_LEN * sizeof(struct e1000_rx_desc));
	e1000w(E1000_RDH, 0);
	e1000w(E1000_RDT, E1000_RX_Q_LEN-1);
	e1000w(E1000_RCTL, E1000_RCTL_EN | E1000_RCTL_LBM_NO | E1000_RCTL_BAM | E1000_RCTL_SZ_1024 | E1000_RCTL_SECRC);

	return 0;
}

// Acknowledge interrupt.
void
e1000_eoi(void)
{
	if (!e1000)
		return;

	e1000r(E1000_ICR); // clear the interrupt
	e1000w(E1000_IMS, E1000_ICR_RXT0);
}

int
e1000_tx(void *data, uint16_t len)
{
	int tail = e1000r(E1000_TDT);

	if (len > E1000_TX_BUFFER_SIZE)
		return -E_INVAL;

	if ((tx_queue[tail].status & 0x1) == 0) // DD bit not set
		return -E_E1000_TX_RING_FULL;

	tx_queue[tail].status &= ~0x1; // clear DD bit

	memcpy((void*)&tx_buffers[tail], data, len);
	tx_queue[tail].length = len;
	e1000w(E1000_TDT, (tail + 1) % E1000_TX_Q_LEN);

	return 0;
}

int
e1000_rx(void *buffer, uint16_t size, int *len)
{
	static int head;

	if ((rx_queue[head].status & 0x1) == 0) // DD bit not set
		return -E_E1000_RX_RING_EMPTY;

	int tail = e1000r(E1000_RDT);

	if (size < rx_queue[head].length)
		return -E_INVAL;

	*len = rx_queue[head].length;
	memcpy(buffer, (void*)&rx_buffers[head], E1000_RX_BUFFER_SIZE);
	rx_queue[head].status &= ~1; // clear DD bit

	e1000w(E1000_RDT, (tail + 1) % E1000_RX_Q_LEN);

	return 0;
}

