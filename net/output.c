#include "ns.h"

extern union Nsipc nsipcbuf;

char ns_output_queue[OUTPUT_QUEUE_SIZE][PGSIZE] __attribute__ ((aligned(PGSIZE)));

static int head, tail;

static void
try_tx_pkts()
{
	int r;

	while (head != tail) {
		struct jif_pkt *pkt = (struct jif_pkt *)ns_output_queue[head];
	
		if ((r = sys_tcp_tx(pkt->jp_data, pkt->jp_len)) == 0)
			head = head + 1 % OUTPUT_QUEUE_SIZE;
		else if (r != E_E1000_TX_RING_FULL)
			panic("try_tx_pkts: %e", r);
	}
}

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	int32_t reqno;
	uint32_t whom;
	int i, perm;
	void *va;
	int r;

	while (1) {
		va = (void *)(OUTPUT_QVA + tail * PGSIZE);

		reqno = ipc_recv((int32_t *) &whom, va, &perm);

		if ((reqno != NSREQ_OUTPUT && reqno != NSREQ_TIMER) || whom != ns_envid) {
			cprintf("Invalid request %x from %08x\n", reqno, whom);
			continue;
		}
		struct jif_pkt *pkt = (struct jif_pkt *)va;
		if (reqno == NSREQ_OUTPUT) {
			if (head == tail) // ns output queue empty
				if ((r = sys_tcp_tx(pkt->jp_data, pkt->jp_len)) == 0) // we were able to send the packet from the first try
					continue;
			memcpy(ns_output_queue[tail], va, PGSIZE);
			tail = tail + 1 % OUTPUT_QUEUE_SIZE;
			if (head == tail)
				panic("SW and HW buffers full! unable to transmit pkgs");
		}

		try_tx_pkts();
	}
}
