#include "ns.h"

extern union Nsipc nsipcbuf;

#define OUTPUT_REQVA (REQVA - PGSIZE)

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver

	int32_t reqno;
	uint32_t whom;
	int i, perm;
	void *va;

	while (1) {
		reqno = ipc_recv((int32_t *) &whom, (void*)OUTPUT_REQVA, &perm);

		if(reqno != NSREQ_OUTPUT || whom != ns_envid) {
			cprintf("Invalid request from %08x\n", whom);
			continue;
		}

		sys_tcp_tx((void*)(OUTPUT_REQVA + sizeof(int)), *(int*)OUTPUT_REQVA);
	}
}
