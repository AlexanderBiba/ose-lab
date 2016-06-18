#include "ns.h"

extern union Nsipc nsipcbuf;

static int head, tail;

static void
try_tx_pkts()
{
	int r;

	while (head != tail) {
		void *va = (void *)(OUTPUT_QVA + head * PGSIZE);
		struct jif_pkt *pkt = (struct jif_pkt *)va;
	
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
		if (reqno == NSREQ_OUTPUT) {
			tail = tail + 1 % OUTPUT_QUEUE_SIZE;
			if (head == tail)
				panic("SW and HW buffers full! unable to transmit pkgs");
		}

		try_tx_pkts();
	}
}
