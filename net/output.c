#include "ns.h"
#include <arch/thread.h>
#include <arch/perror.h>

/* errno to make lwIP happy */
int errno;

extern union Nsipc nsipcbuf;

static bool buse[OUTPUT_QUEUE_SIZE];

static void
usleep(int usec)
{
        unsigned now = sys_time_msec();
        unsigned end = now + usec;

        if ((int)now < 0 && (int)now > -MAXERROR)
                panic("sys_time_msec: %e", (int)now);
        if (end < now)
                panic("usleep: wrap");

        while (sys_time_msec() < end)
                sys_yield();
}

static void *
get_buffer(void) {
	void *va;

	int i;
	for (i = 0; i < OUTPUT_QUEUE_SIZE; i++)
		if (!buse[i]) break;

	if (i == OUTPUT_QUEUE_SIZE) {
		panic("NS: buffer overflow");
		return 0;
	}

	va = (void *)(OUTPUT_REQVA + i * PGSIZE);

	return va;
}

static void
mark_buffer(void *va) {
	int i = ((uint32_t)va - OUTPUT_REQVA) / PGSIZE;
	buse[i] = 1;
}

static void
put_buffer(void *va) {
	int i = ((uint32_t)va - OUTPUT_REQVA) / PGSIZE;
	buse[i] = 0;
}

static void
tx_thread(uint32_t arg)
{
	int i;
	int r;
	cprintf("hi\n");

	while(1) {
		usleep(1);
		for (i = 0; i < OUTPUT_QUEUE_SIZE; i++) {	//	iterate over the queue and find pkts to transmit
			
			if (!buse[i])
				continue;

			void *va = (void *)(OUTPUT_REQVA + i * PGSIZE);
			struct jif_pkt *pkt = (struct jif_pkt *)va;

			if ((r = sys_tcp_tx(pkt->jp_data, pkt->jp_len)) == 0)
				put_buffer(va);	//	pkt transmitted successfully, we can free the buffer, otherwise continue
			else if (r != E_E1000_TX_RING_FULL)
				panic("output.c: %e", r);
		}
		cprintf("hi2\n");
		thread_yield();
	}
}

static void
ipc_rcv_thread(uint32_t ns_envid)
{
	int32_t reqno;
	uint32_t whom;
	int i, perm;
	void *va;
	int r;

	while (1) {
		va = get_buffer();

		reqno = ipc_recv((int32_t *) &whom, va, &perm);

		if (reqno != NSREQ_OUTPUT || whom != ns_envid) {
			cprintf("Invalid request from %08x\n", whom);
			continue;
		}

		mark_buffer(va);
		thread_yield();
	}
}

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver

	int r;

	if ((r = thread_create(0, "tx_thread", &tx_thread, 0)) < 0)
		panic("cannot create tx_thread: %s", e2s(r));
	if ((r = thread_create(0, "ipc_rcv_thread", &ipc_rcv_thread, ns_envid)) < 0)
		panic("cannot create ipc_rcv_thread: %s", e2s(r));

	thread_yield();
}
