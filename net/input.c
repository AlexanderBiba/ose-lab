#include "ns.h"

extern union Nsipc nsipcbuf;

char ns_input_queue[INPUT_QUEUE_SIZE][PGSIZE] __attribute__ ((aligned(PGSIZE)));

static void keep_alive()
{
	while (1) sys_yield();
}

void
input(envid_t ns_envid)
{
	binaryname = "ns_input";

	int i, r;
	int curr = 0;

	envid_t ka_envid;

	ka_envid = fork();
	if (ka_envid < 0)
		panic("error forking");
	else if (ka_envid == 0) {
		keep_alive();
		return;
	}

	// LAB 6: Your code here:
	// 	- read a packet from the device driver
	//	- send it to the network server
	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.
	while (1) {
		if ((r = sys_tcp_rx(nsipcbuf.pkt.jp_data, PGSIZE, &nsipcbuf.pkt.jp_len)) < 0)
			panic("input: %e", r);

		memcpy(ns_input_queue[curr], &nsipcbuf, sizeof(union Nsipc));
		ipc_send(ns_envid, NSREQ_INPUT, ns_input_queue[curr], PTE_P | PTE_U);

		curr = (curr + 1) % INPUT_QUEUE_SIZE;
	}
}
