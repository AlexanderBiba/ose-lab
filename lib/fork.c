// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	int perm = PGOFF(uvpt[PGNUM(addr)]);

	if (!(err & FEC_WR))
		panic("Page fault for read access !");

	if ((perm & PTE_COW) == 0)
		panic("Page fault for a non-cow page !");

	if ((perm & PTE_P) == 0)
		panic("Page fault for a non-present page !");

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	envid_t envid = sys_getenvid();

	int newperm = perm & PTE_SYSCALL;
	void *va = (void *)ROUNDDOWN((uintptr_t)addr, PGSIZE);

	if ((r = sys_page_alloc(envid, PFTEMP, newperm | PTE_W)) < 0)
		panic("sys_page_alloc: %e", r);

	memmove(PFTEMP, va, PGSIZE);

	if ((r = sys_page_map(envid, PFTEMP, envid, va, newperm | PTE_W)) < 0)
		panic("sys_page_map: %e", r);

}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	// LAB 4: Your code here.
	int r;
	int perm = PGOFF(uvpt[pn]);
	void *va = (void *) (pn * PGSIZE);

	// just copy the mapping
	if ((perm & PTE_SHARE) == PTE_SHARE) {
		perm &= PTE_SYSCALL;
		if ((r = sys_page_map(0, va, envid, va, perm)) < 0)
			panic("sys_page_map failed: %e", r);
		return 0;
	}

	// copy on write
	int newperm =	(perm & PTE_W)	 == PTE_W ||
			(perm & PTE_COW) == PTE_COW ? 
			(perm | PTE_COW) & ~PTE_W :
			perm;

	newperm &= PTE_SYSCALL;

	if ((r = sys_page_map(0, va, envid, va, newperm)) < 0)
		panic("sys_page_map failed: %e", r);

	if ((r = sys_page_map(0, va, 0, va, newperm)) < 0)
		panic("sys_page_map failed: %e", r);

	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	set_pgfault_handler(pgfault);
	envid_t child_envid = sys_exofork();

	if (child_envid < 0)
		panic("sys_exofork failed: %e", child_envid);

	if (child_envid != 0) {	//	parent process
		uintptr_t va;
		int r;

		for (va = 0; va < UXSTACKTOP - PGSIZE; va += PGSIZE) {
			pde_t *pde = (pde_t *) &uvpd[PDX(va)];
			pte_t *pte = (pte_t *) &uvpt[PGNUM(va)];

			if ((*pde & PTE_P) && (*pte & PTE_P))
				duppage(child_envid, PGNUM(va));
		}

		if ((r = sys_page_alloc(child_envid, (void *) UXSTACKTOP - PGSIZE, PTE_P | PTE_U | PTE_W)) < 0)
			panic("sys_page_alloc failed: %e", r);

		if ((r = sys_env_set_pgfault_upcall(child_envid, thisenv->env_pgfault_upcall)) < 0)
			panic("sys_env_set_pgfault_upcall failed: %e", r);

		if ((r = sys_env_set_status(child_envid, ENV_RUNNABLE)) < 0)
			panic("sys_env_set_status: %e", r);

	} else {	//	child process
		thisenv = &envs[ENVX(sys_getenvid())];
		assert (thisenv->env_id == sys_getenvid());
	}

	return child_envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
