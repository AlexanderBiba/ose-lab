// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/pmap.h>
#include <kern/kdebug.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line
#define FN_NAME_MAX_LEN	256


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "Display stack backtrace", mon_backtrace },
	{ "showmappings", "Display physical page mappings", mon_showmappings },
	{ "setmappings", "Set physical page mappings", mon_setmappings },
	{ "clearmappings", "Clear physical page mappings", mon_clearmappings },
	{ "changemappingsperm", "Clear physical page mappings", mon_changemappingsperm },
	{ "dumpmem", "Dump memory contents", mon_dumpmem },
	{ "showallocs", "Display allocated address spaces", mon_showallocs },
};
#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

void print_curr_trace() {
	uint32_t prev_ebp;
	uint32_t prev_eip;
	uint32_t prev_args[5];
	struct Eipdebuginfo info;

	__asm __volatile("movl (%%ebp),%0" : "=r" (prev_ebp));
	__asm __volatile("movl 0x4(%%ebp),%0" : "=r" (prev_eip));
	__asm __volatile("movl 0x8(%1),%0" : "=r" (prev_args[0]) : "r" (prev_ebp));
	__asm __volatile("movl 0xC(%1),%0" : "=r" (prev_args[1]) : "r" (prev_ebp));
	__asm __volatile("movl 0x10(%1),%0" : "=r" (prev_args[2]) : "r" (prev_ebp));
	__asm __volatile("movl 0x14(%1),%0" : "=r" (prev_args[3]) : "r" (prev_ebp));
	__asm __volatile("movl 0x18(%1),%0" : "=r" (prev_args[4]) : "r" (prev_ebp));

	cprintf("  ebp %08x eip %08x args %08x %08x %08x %08x %08x\n", prev_ebp, prev_eip, prev_args[0], prev_args[1], prev_args[2], prev_args[3], prev_args[4]);

	if(!debuginfo_eip(prev_eip, &info)){
		uint32_t fn_addr = prev_eip-(uint32_t)info.eip_fn_addr;
		cprintf("\t%s:%d: %.*s+%d\n", info.eip_file, info.eip_line, info.eip_fn_namelen, info.eip_fn_name, fn_addr);
	}
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	extern char bootstacktop[];

	int i;
	int j;
	uint32_t prev_ebp;
	uint32_t prev_eip;
	uint32_t prev_args[5];
	uint32_t curr_bp;
	struct Eipdebuginfo info;
	char dbg_fn_name[FN_NAME_MAX_LEN];

	cprintf("Stack backtrace:\n");

	print_curr_trace();

	__asm __volatile("mov %%ebp, %0" : "=r" (curr_bp));

	while(curr_bp != (uint32_t)bootstacktop-8) {
		__asm __volatile("movl (%1),%0" : "=r" (prev_ebp) : "r" (curr_bp));
		__asm __volatile("movl 0x4(%1),%0" : "=r" (prev_eip) : "r" (curr_bp));
		__asm __volatile("movl 0x8(%1),%0" : "=r" (prev_args[0]) : "r" (prev_ebp));
		__asm __volatile("movl 0xC(%1),%0" : "=r" (prev_args[1]) : "r" (prev_ebp));
		__asm __volatile("movl 0x10(%1),%0" : "=r" (prev_args[2]) : "r" (prev_ebp));
		__asm __volatile("movl 0x14(%1),%0" : "=r" (prev_args[3]) : "r" (prev_ebp));
		__asm __volatile("movl 0x18(%1),%0" : "=r" (prev_args[4]) : "r" (prev_ebp));

		cprintf("  ebp %08x eip %08x args %08x %08x %08x %08x %08x\n", prev_ebp, prev_eip, prev_args[0], prev_args[1], prev_args[2], prev_args[3], prev_args[4]);

		if(!debuginfo_eip(prev_eip, &info)){
			uint32_t fn_addr = prev_eip-(uint32_t)info.eip_fn_addr;
			cprintf(" \t%s:%d: %.*s+%d\n", info.eip_file, info.eip_line, info.eip_fn_namelen, info.eip_fn_name, fn_addr);
		}

		__asm __volatile("mov (%1), %0" : "=r" (curr_bp) : "r" (curr_bp));
	}

	return 0;
}

int 
mon_showmappings(int argc, char **argv, struct Trapframe *tf) {
	if(argc != 3) {
		cprintf("showmappings usage: showmappings 0x<start_addr> 0x<end_addr>\n");
		return -1;
	}

	uintptr_t start = strtol(argv[1], NULL, 16), end = strtol(argv[2], NULL, 16);
	pte_t *pte = NULL;
	struct PageInfo *pp = NULL;
	uintptr_t va = 0;

	void* a = (void*)ROUNDDOWN(start, PGSIZE);
	void* last = (void*)ROUNDDOWN(end, PGSIZE);
	for (; a <= last; a += PGSIZE) {
		pp = page_lookup(kern_pgdir, (void*)a, &pte);
		cprintf("va 0x%08x is ", a);
		if(pp)
			cprintf("mapped to pa 0x%08x with permissions %d, page->pp_ref = %d\n", page2pa(pp), PGOFF(*pte), pp->pp_ref);	//	PGOFF gives us 12 LSBs
		else
			cprintf("unmapped\n");
	}
	return 0;
}

int 
mon_setmappings(int argc, char **argv, struct Trapframe *tf) {
	if(argc != 5) {
		cprintf("setmappings usage: setmappings 0x<virt_start_addr> 0x<virt_end_addr> 0x<phys_addr> 0x<permissions>\n");
		return -1;
	}

	uintptr_t start = strtol(argv[1], NULL, 16), end = strtol(argv[2], NULL, 16);
	physaddr_t pa = strtol(argv[3], NULL, 16);
	uint32_t perm = strtol(argv[4], NULL, 16);

	pte_t *pte = NULL;
	struct PageInfo *pp = NULL;

	void* a = (void*)ROUNDDOWN(start, PGSIZE);
	void* last = (void*)ROUNDDOWN(end, PGSIZE);
	for(;a <= last; a += PGSIZE, pa += PGSIZE) {
		if(page_insert(kern_pgdir, pa2page(pa), a, perm) != 0)
			panic("page insert returned no more mem !");
		cprintf("Mapping va 0x%08x to pa 0x%08x\n", a, pa);
	}

	return 0;
}

int 
mon_clearmappings(int argc, char **argv, struct Trapframe *tf) {
	if(argc != 3) {
		cprintf("clearmappings usage: clearmappings 0x<start_addr> 0x<end_addr>\n");
		return -1;
	}

	uintptr_t start = strtol(argv[1], NULL, 16), end = strtol(argv[2], NULL, 16);
	pte_t *pte = NULL;
	struct PageInfo *pp = NULL;
	uintptr_t va = 0;

	void* a = (void*)ROUNDDOWN(start, PGSIZE);
	void* last = (void*)ROUNDDOWN(end, PGSIZE);
	for (; a <= last; a += PGSIZE) {
		pp = page_lookup(kern_pgdir, (void*)a, &pte);
		cprintf("va 0x%08x is ", a);
		if(pp) {
			page_remove(kern_pgdir, a);
			cprintf("mapped to pa 0x%08x, unmapping..\n", page2pa(pp));
		}
		else
			cprintf("unmapped\n");
	}
	return 0;
}

int 
mon_changemappingsperm(int argc, char **argv, struct Trapframe *tf) {
	if(argc != 4) {
		cprintf("changemappingsperm usage: changemappingsperm 0x<virt_start_addr> 0x<virt_end_addr> 0x<permissions>\n");
		return -1;
	}

	uintptr_t start = strtol(argv[1], NULL, 16), end = strtol(argv[2], NULL, 16);
	uint32_t perm = strtol(argv[3], NULL, 16);

	pte_t *pte = NULL;
	struct PageInfo *pp = NULL;

	void* a = (void*)ROUNDDOWN(start, PGSIZE);
	void* last = (void*)ROUNDDOWN(end, PGSIZE);
	for(;a <= last; a += PGSIZE) {
		pp = page_lookup(kern_pgdir, (void*)a, &pte);
		if(!pp)
			cprintf("va not mapped\n");
		else if(page_insert(kern_pgdir, pp, a, perm) != 0)
			panic("page insert returned no more mem !");
		else
			cprintf("Changing permissions of va 0x%08x to 0x%08x\n", a, perm);
	}

	return 0;
}

int 
mon_dumpmem(int argc, char **argv, struct Trapframe *tf) {
	if(argc != 4) {
		cprintf("dumpmem usage: dumpmem <phys/virt> 0x<start_addr> 0x<end_addr>\n");
		return -1;
	}

	bool phys = (strcmp(argv[1], "phys") == 0);	//	true if argv[1] == "phys"
	bool virt = (strcmp(argv[1], "virt") == 0);	//	true if argv[1] == "virt"
	if(!phys && !virt) {
		cprintf("dumpmem usage: dumpmem <phys/virt> 0x<start_addr> 0x<end_addr>\n");
		return -1;
	}
	uint32_t start = strtol(argv[2], NULL, 16), end = strtol(argv[3], NULL, 16);

	pte_t *pte = NULL;

	if(phys) {
		physaddr_t a = (physaddr_t)start;
		physaddr_t last = (physaddr_t)end;
		struct PageInfo *pp = pa2page(a);

		for(; a <= last; a++) {
			if(a % PGSIZE == 0)
				pp = pa2page(a);
			cprintf("*0x%08x = 0x%08x\n", a, pp[PGOFF(a)]);
		}
	}
	else if(virt) {
		uintptr_t a = (uintptr_t)start;
		uintptr_t last = (uintptr_t)end;
		struct PageInfo *pp = page_lookup(kern_pgdir, (void*)a, NULL);

		for(; a <= last; a++) {
			if(a % PGSIZE == 0)
				pp = page_lookup(kern_pgdir, (void*)a, NULL);
			if(!pp)
				cprintf("*0x%08x unmapped\n", a);
			else
				cprintf("*0x%08x = 0x%08x\n", a, pp[PGOFF(a)]);
		}
	}

	return 0;
}

int
mon_showallocs(int argc, char **argv, struct Trapframe *tf) {
	//if(argc != 4) {
	//	cprintf("showallocs usage: showallocs\n");
	//	return -1;
	//}
	uint32_t i = 0;
	uint32_t a = 0;
	pte_t *pte;

	for(; i < PGNUM(0xFFFFFFFF);) {
		if(page_lookup(kern_pgdir, (void*)a, NULL) == 0) {
			while(i < PGNUM(0xFFFFFFFF) && page_lookup(kern_pgdir, (void*)a, NULL) == 0) {
				i++;
				a = (i << PTXSHIFT);
			}
		}
		else if(page_lookup(kern_pgdir, (void*)a, NULL) == (void*)-1) {
			cprintf("addresses 0x%08x - ", a);
			while(i < PGNUM(0xFFFFFFFF) && page_lookup(kern_pgdir, (void*)a, NULL) == (void*)-1) {
				i++;
				a = (i << PTXSHIFT);
			}
			cprintf("0x%08x are mapped to invalid phys addresses\n", a);
		} 
		else {
			cprintf("addresses 0x%08x - ", a);
			while(i < PGNUM(0xFFFFFFFF) && page_lookup(kern_pgdir, (void*)a, NULL) != (void*)-1 && page_lookup(kern_pgdir, (void*)a, NULL) != 0) {
				i++;
				a = (i << PTXSHIFT);
			}
			cprintf("0x%08x are allocated\n", a);
		}
	}
	return 0;
}

/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");


	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
