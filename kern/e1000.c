#include <kern/e1000.h>
#include <kern/pmap.h>
#include <inc/string.h>
#include <inc/error.h>

#define TX_Q_LEN	32
#define BUFFER_SIZE	32		//	TODO: should be at least 2^16 bytes ?
#define TX_BUFFER_SIZE	BUFFER_SIZE

struct tx_desc
{
        uint64_t addr;
        uint16_t length;
        uint8_t cso;
        uint8_t cmd;
        uint8_t status;
        uint8_t css;
        uint16_t special;
} __attribute__ ((packed, aligned(16)));

typedef struct { 
	uint8_t x[TX_BUFFER_SIZE]; 
} __attribute__((packed)) buffer_t;


static inline physaddr_t
kva2pa(void *kva)
{
	return page2pa(page_lookup(kern_pgdir, kva, NULL)) | PGOFF(kva);
}

volatile uintptr_t *e1000;
volatile struct tx_desc tx_queue[TX_Q_LEN];
volatile buffer_t tx_buffers[TX_Q_LEN];

static void
e1000w(int index, int value)
{
	//cprintf("E1000: e1000w: idx 0x%08x val 0x%08x\n", index, value);
	e1000[index/4] = value;
}

static uint32_t
e1000r(int index)
{
	uint32_t value = e1000[index/4];
	//cprintf("E1000: e1000r: idx 0x%08x val 0x%08x\n", index, value);
	return value;
}

int
e1000_init(struct pci_func *pcif) 
{
	int i;

	// ex 2
	pci_func_enable(pcif);

	// ex 3
	e1000 = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);
	e1000r(E1000_STATUS);	//	for debug

	// ex 4
	for (i = 0; i < TX_Q_LEN; i++) {
		tx_queue[i].addr = kva2pa((void*)&tx_buffers[i]);
		tx_queue[i].cmd |= 0x9;	//	set RS & EOP bits
		tx_queue[i].status |= 0x1;	//	set DD bit TODO: buggy ?
	}

	e1000w(E1000_TDBAL, kva2pa(&tx_queue));
	e1000w(E1000_TDBAH, 0);	//	for 32 bit system
	e1000w(E1000_TDLEN, TX_Q_LEN * sizeof(struct tx_desc));
	e1000w(E1000_TDH, 0);
	e1000w(E1000_TDT, 0);
	e1000w(E1000_TCTL, E1000_TCTL_EN | E1000_TCTL_PSP | (E1000_TCTL_CT & 0x10) | (E1000_TCTL_COLD & 0x40));
	uint32_t ipgt = 10;
	uint32_t ipgr2 = 6;
	uint32_t ipgr1 = (2*ipgr2)/3;
	e1000w(E1000_TIPG, ipgr2 << 20 | ipgr1 << 10 | ipgt << 0);

	return 0;
}

int
e1000_tx(void *data, uint16_t len)
{
	int tail = e1000r(E1000_TDT);

	if (len > TX_BUFFER_SIZE)
		return -E_INVAL;

	if ((tx_queue[tail].status & 1) == 0)	//	DD bit not set
		return -E_E1000_TX_RING_FULL;

	tx_queue[tail].status &= ~1;	//	clear DD bit TODO: buggy ?

	memcpy((void*)&tx_buffers[tail], data, len);
	tx_queue[tail].length = len;
	e1000w(E1000_TDT, (tail + 1) % TX_Q_LEN);

	return 0;
}
