// Fork a binary tree of processes and display their structure.

#include <inc/lib.h>

static int prio;

void
umain(int argc, char **argv)
{
	int id;

	if ((id = fork()) == 0) {

		cprintf("child\n");
		sys_yield();
		cprintf("child\n");
		sys_yield();
		cprintf("child\n");
		exit();
	} else {
		// setting both envs to the same prio results in RR
		sys_env_set_sched_prio(thisenv->env_id, 5);
		sys_env_set_sched_prio(id, 5);

		sys_yield();
		cprintf("parent\n");
		sys_yield();
		cprintf("parent\n");
		sys_yield();
		cprintf("parent\n");
		sys_yield();
	}

	if ((id = fork()) == 0) {

		cprintf("child\n");
		sys_yield();
		cprintf("child\n");
		sys_yield();
		cprintf("child\n");
		exit();
	} else {
		// setting child env to a higher prio
		sys_env_set_sched_prio(thisenv->env_id, 3);
		sys_env_set_sched_prio(id, 5);

		sys_yield();
		cprintf("parent\n");
		sys_yield();
		cprintf("parent\n");
		sys_yield();
		cprintf("parent\n");
		sys_yield();
	}
}

