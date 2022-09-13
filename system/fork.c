/* fork.c - exit */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  fork  -  Fork a process to start running a function on x86
 *------------------------------------------------------------------------
 */
pid32	fork(void)
{
	uint32		savsp, *pushsp;
    uint32	    ssize;	    /* Stack size in bytes		*/
	intmask 	mask;    	/* Interrupt mask		*/
    pid32       parent_pid; /* Stores parent process id */
	pid32		pid;		/* Stores new process id for child	*/
    struct	procent	*parent_prptr;  /* Pointer to proc. table entry for parent */
	struct	procent	*prptr;		/* Pointer to proc. table entry for child   */
	int32		i;          /*  */
    char* parent_saddr;
	uint32* parent_bp;
	uint32		*saddr;		/* Stack address		*/
	unsigned long *parent_bp_register;		/* Stack Base Pointer Register*/
	signed long stack_offset;
    
	mask = disable();

    parent_pid = (pid32)getpid();
    parent_prptr = &proctab[parent_pid];
    parent_saddr = parent_prptr->prstkbase;

	ssize = parent_prptr->prstklen;

    if (ssize < MINSTK)
		ssize = MINSTK;
	ssize = (uint32) roundmb(ssize);
	if ( ((pid=newpid()) == SYSERR) ||
	     ((saddr = (uint32 *)getstk(ssize)) == (uint32 *)SYSERR) ) {
		restore(mask);
		return SYSERR;
	}

	prcount++;
	prptr = &proctab[pid];

	/* Initialize process table entry for new process */
	prptr->prstate = PR_READY;	/* Initial state is ready	*/  // ASK ABOUT THIS!!!
	prptr->prprio = parent_prptr->prprio;
	prptr->prstkbase = (char *)saddr;
	prptr->prstklen = ssize;
	prptr->prname[PNMLEN-1] = NULLCH;
	for (i=0 ; i<PNMLEN-1 && (prptr->prname[i]=parent_prptr->prname[i])!=NULLCH; i++)
		;
	prptr->prsem = -1;
	prptr->prparent = parent_pid;
	prptr->prhasmsg = FALSE;

	/* Set up stdin, stdout, and stderr descriptors for the shell	*/
	prptr->prdesc[0] = CONSOLE;
	prptr->prdesc[1] = CONSOLE;
	prptr->prdesc[2] = CONSOLE;

	/* Record point in time at which the process is created */

	prptr->prtime = ctr1000;

	/* Initialize stack as if the process was called		*/

	*saddr = STACKMAGIC;
    savsp = (uint32)saddr;

	/* Retrieve the current stack base pointer */

	asm("movl %%ebp, %0\n" :"=r"(parent_bp_register));
	parent_bp = (uint32 *)parent_bp_register;
	kprintf("Parent base: %X\nChild base: %X\n", parent_bp, saddr);

	/* Determine memory offset between parent and child stacks */

	stack_offset = (long)savsp - (long)((uint32)(parent_prptr->prstkbase));
	kprintf("%X = %d\n", stack_offset, stack_offset);

    /* Copy the contents of the parent stack into the child stack */

	kprintf("Copying parent to child\n");
    while (parent_saddr>*parent_bp) {	// Where should we stop copying the stack?
        *--saddr = *--parent_saddr;
    }
	
	/* Update the base pointers for every stack frame */
	
	kprintf("Updating base pointers\n");
	kprintf("Parent: %X -> %X\n", parent_bp, *parent_bp);
	parent_bp = *parent_bp;
	kprintf("Parent: %X -> %X\n", parent_bp, *parent_bp);
	while (*parent_bp != STACKMAGIC) {
		 kprintf("Child: %X -> %X\n", (uint32 *)((uint32)parent_bp + stack_offset), (uint32 *)((uint32)(*parent_bp) + stack_offset));
		 *(uint32 *)((uint32)parent_bp + stack_offset) = (uint32 *)((uint32)(*parent_bp) + stack_offset); 
		 parent_bp = *parent_bp;
		 kprintf("Parent: %X -> %X\n", parent_bp, *parent_bp);
	}
	kprintf("Child: %X -> %X\n", (uint32 *)((uint32)parent_bp + stack_offset), (uint32 *)((uint32)(*parent_bp) + stack_offset));

	kprintf("Setting up context switch\n");

	/* The following entries on the stack must match what ctxsw	*/
	/*   expects a saved process state to contain: ret address,	*/
	/*   ebp, interrupt mask, flags, registers, and an old SP	*/

	//*--saddr = (long)funcaddr;	/* Make the stack look like it's*/
	kprintf("%X\n", (long)(*(uint32 *)((uint32)parent_bp_register+1)));
	*--saddr = (long)(*(uint32 *)((uint32)parent_bp_register+1));	/* Make the stack look like it's*/
					/*   half-way through a call to	*/
					/*   ctxsw that "returns" to the*/
					/*   new process		*/
	kprintf("%X\n", (long)((uint32)(*parent_bp_register) + stack_offset));
	*--saddr = (long)((uint32)(*parent_bp_register) + stack_offset);		/* This will be register ebp	*/
					/*   for process exit		*/
	savsp = (uint32) saddr;		/* Start of frame for ctxsw	*/
	*--saddr = 0x00000200;		/* New process runs with	*/
					/*   interrupts enabled		*/

	/* Basically, the following emulates an x86 "pushal" instruction*/

	*--saddr = NPROC;			/* %eax */
	//*--saddr = (long)(*(uint32 *)((uint32)parent_bp_register+3));			/* %ecx */
	*--saddr = 0;			/* %ecx */
	*--saddr = 0;			/* %edx */
	*--saddr = 0;			/* %ebx */
	*--saddr = 0;			/* %esp; value filled in below	*/
	pushsp = saddr;			/* Remember this location	*/
	*--saddr = savsp;		/* %ebp (while finishing ctxsw)	*/
	*--saddr =  0;			/* %esi */
	*--saddr =  0;			/* %edi */
	*pushsp = (unsigned long) (prptr->prstkptr = (char *)saddr);

	kprintf("Add child to ready list\n");

    insert(pid, readylist, prptr->prprio);  // insert child into the ready list
	restore(mask);
	kprintf("Return from fork\n");
	return pid;
}