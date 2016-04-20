/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>



###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

/*

// first challenge: this is what the definition should look like
// ISTRAP and DPL values are incorrect, correct values can be found at trap_init()
//
//	 NAME		NUM		ISTRAP?	DPL	EC?
DEF_TRAP(DIVIDE,	T_DIVIDE,	1, 	0, 	0);
DEF_TRAP(DEBUG,		T_DEBUG,	1, 	0, 	0);
DEF_TRAP(NMI,		T_NMI,		1, 	0, 	0);
DEF_TRAP(BRKPT,		T_BRKPT,	1, 	0, 	0);
DEF_TRAP(OFLOW,		T_OFLOW,	1, 	0, 	0);
DEF_TRAP(BOUND,		T_BOUND,	1, 	0, 	0);
DEF_TRAP(ILLOP,		T_ILLOP,	1, 	0, 	0);
DEF_TRAP(DEVICE,	T_DEVICE,	1, 	0, 	0);
DEF_TRAP(DBLFLT,	T_DBLFLT,	1, 	0, 	1);
DEF_TRAP(TSS,		T_TSS,		1, 	0, 	1);
DEF_TRAP(SEGNP,		T_SEGNP,	1, 	0, 	1);
DEF_TRAP(STACK,		T_STACK,	1, 	0, 	1);
DEF_TRAP(GPFLT,		T_GPFLT,	1, 	0, 	1);
DEF_TRAP(PGFLT,		T_PGFLT,	1, 	0, 	1);
DEF_TRAP(FPERR,		T_FPERR,	1, 	0, 	0);
DEF_TRAP(ALIGN,		T_ALIGN,	1, 	0, 	1);	//	does not appear in 80386 prog manual
DEF_TRAP(MCHK,		T_MCHK,		1, 	0, 	0);	//	does not appear in 80386 prog manual
DEF_TRAP(SIMDERR,	T_SIMDERR,	1, 	0, 	0);	//	does not appear in 80386 prog manual

DEF_TRAP(SYSCALL,	T_SYSCALL,	1, 	0, 	1);
DEF_TRAP(DEFAULT,	T_DEFAULT,	1, 	0, 	1);

*/ 

TRAPHANDLER_NOEC(DIVIDE,	T_DIVIDE);
TRAPHANDLER_NOEC(DEBUG,		T_DEBUG);
TRAPHANDLER_NOEC(NMI,		T_NMI);
TRAPHANDLER_NOEC(BRKPT,		T_BRKPT);
TRAPHANDLER_NOEC(OFLOW,		T_OFLOW);
TRAPHANDLER_NOEC(BOUND,		T_BOUND);
TRAPHANDLER_NOEC(ILLOP,		T_ILLOP);
TRAPHANDLER_NOEC(DEVICE,	T_DEVICE);
TRAPHANDLER	(DBLFLT,	T_DBLFLT);
TRAPHANDLER	(TSS,		T_TSS);
TRAPHANDLER	(SEGNP,		T_SEGNP);
TRAPHANDLER	(STACK,		T_STACK);
TRAPHANDLER	(GPFLT,		T_GPFLT);
TRAPHANDLER	(PGFLT,		T_PGFLT);
TRAPHANDLER_NOEC(FPERR,		T_FPERR);
TRAPHANDLER	(ALIGN,		T_ALIGN);	//	does not appear in 80386 prog manual
TRAPHANDLER_NOEC(MCHK,		T_MCHK);	//	does not appear in 80386 prog manual
TRAPHANDLER_NOEC(SIMDERR,	T_SIMDERR);	//	does not appear in 80386 prog manual

TRAPHANDLER	(SYSCALL,	T_SYSCALL);
TRAPHANDLER	(DEFAULT,	T_DEFAULT);

/*
 * Lab 3: Your code here for _alltraps
 */

.globl _alltraps
_alltraps:
	pushl %ds;
	pushl %es;
	pushal;
	pushl %esp;
	call trap;
