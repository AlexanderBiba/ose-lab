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
e1000w(int addr, int data)
{
	if (!e1000)
		return;

#ifdef LOG_REGS
	cprintf("E1000: e1000w: addr 0x%08x data 0x%08x\n", addr, data);
#endif
	e1000[addr/4] = data;
}

static uint32_t
e1000r(int addr)
{
	if (!e1000)
		return 0;

	uint32_t data = e1000[addr/4];
#ifdef LOG_REGS
	cprintf("E1000: e1000r: addr 0x%08x data 0x%08x\n", addr, data);
#endif
	return data;
}

static int
e1000_eeprom_r(uint8_t addr)
{
	if (!e1000)
		return 0;

	e1000w(E1000_EERD, addr << E1000_EEPROM_RW_ADDR_SHIFT | E1000_EEPROM_RW_REG_START);
	while (!(e1000r(E1000_EERD) & E1000_EEPROM_RW_REG_DONE));
	uint16_t data = (e1000r(E1000_EERD) >> E1000_EEPROM_RW_REG_DATA);
#ifdef LOG_REGS
	cprintf("E1000: e1000_eeprom_r: addr 0x%02x data 0x%04x\n", addr, data);
#endif
	return data;
}

void
e1000_get_hwaddr(uint8_t buffer[6])
{
	if (!e1000)
		return;

	memcpy(buffer, e1000_hwaddr, 6);
}

// Acknowledge interrupt.
void
e1000_eoi(void)
{
	if (!e1000)
		return;

	e1000r(E1000_ICR); // clear the interrupt
}

int
e1000_init(struct pci_func *pcif) 
{
	int i;

	// enable pci device and get BARs and sizes
	pci_func_enable(pcif);

	// map registers in memory
	e1000 = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);

	// enable irq line
	irq_setmask_8259A(irq_mask_8259A & ~(1 << pcif->irq_line));

	// get mac address from EEPROM
	((uint16_t*) e1000_hwaddr)[0] = e1000_eeprom_r(0);
	((uint16_t*) e1000_hwaddr)[1] = e1000_eeprom_r(1);
	((uint16_t*) e1000_hwaddr)[2] = e1000_eeprom_r(2);

	// init tx descriptors
	for (i = 0; i < E1000_TX_Q_LEN; i++) {
		tx_queue[i].buffer_addr = kva2pa((void*)&tx_buffers[i]);
		tx_queue[i].status = E1000_TXD_STAT_DD;
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
	e1000w(E1000_RAL0, (e1000_hwaddr[3] << 24) | (e1000_hwaddr[2] << 16) | (e1000_hwaddr[1] << 8) | (e1000_hwaddr[0] << 0));
	e1000w(E1000_RAH0, /* set addr valid bit */  (0x1             << 31) | (e1000_hwaddr[5] << 8) | (e1000_hwaddr[4] << 0));
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

int
e1000_tx(void *data, uint16_t len)
{
	if (!e1000)
		return -E_INVAL;

	int tail = e1000r(E1000_TDT);

	if (len > E1000_TX_BUFFER_SIZE)
		return -E_INVAL;

	if ((tx_queue[tail].status & E1000_TXD_STAT_DD) == 0)
		return -E_E1000_TX_RING_FULL;

	tx_queue[tail].status &= ~E1000_TXD_STAT_DD;
	tx_queue[tail].cmd = (E1000_TXD_CMD_RS | E1000_TXD_CMD_EOP) >> 24;

	memcpy((void*)&tx_buffers[tail], data, len);
	tx_queue[tail].length = len;
	e1000w(E1000_TDT, (tail + 1) % E1000_TX_Q_LEN);

	return 0;
}

int
e1000_rx(void *buffer, uint16_t size, int *len)
{
	if (!e1000)
		return -E_INVAL;

	static int head;

	if ((rx_queue[head].status & E1000_RXD_STAT_DD) == 0)
		return -E_E1000_RX_RING_EMPTY;

	int tail = e1000r(E1000_RDT);

	if (size < rx_queue[head].length)
		return -E_INVAL;

	*len = rx_queue[head].length;
	memcpy(buffer, (void*)&rx_buffers[head], E1000_RX_BUFFER_SIZE);
	rx_queue[head].status &= ~E1000_RXD_STAT_DD;

	head = (head + 1) % E1000_RX_Q_LEN;
	e1000w(E1000_RDT, (tail + 1) % E1000_RX_Q_LEN);

	return 0;
}

