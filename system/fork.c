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
    uint32 *parent_saddr, *parent_stkbase;
	uint32 *parent_bp, *prev_parent_bp, *parent_sp;
	uint32		*saddr;		/* Stack address		*/
	uint32		*child_eip;
	unsigned long *parent_bp_register, *parent_sp_register;		/* Stack Base Pointer Register*/
	signed long stack_offset;
    
	mask = disable();

    parent_pid = (pid32)getpid();
    parent_prptr = &proctab[parent_pid];
    parent_saddr = parent_prptr->prstkbase;
	parent_stkbase = parent_saddr;

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
	//sync_printf("Parent base: %X\nChild base: %X\n", parent_bp, saddr);

	/* Determine memory offset between parent and child stacks */

	stack_offset = (long)savsp - (long)((uint32)(parent_prptr->prstkbase));
	//sync_printf("%X = %d\n", stack_offset, stack_offset);

    /* Copy the contents of the parent stack into the child stack */

	//stacktrace(getpid());
	//sync_printf("Copying parent to child\n");

	asm("movl %%esp, %0\n" :"=r"(parent_sp_register));
	parent_sp = (uint32 *)parent_sp_register;
	//sync_printf("Parent bp: %X\nParent sp: %X\n", parent_bp, parent_sp);

    while (parent_saddr>parent_sp) {
        *--saddr = *--parent_saddr;
    }
	
	/* Update the base pointers for every stack frame */
	
	//sync_printf("Updating base pointers\n");
	//sync_printf("DEBUG 0\n");
	while (*parent_bp != STACKMAGIC) {
		//sync_printf("DEBUG 1\n");
		*(uint32 *)((uint32)parent_bp + stack_offset) = (uint32 *)((uint32)(*parent_bp) + stack_offset); 
		//sync_printf("DEBUG 2\n");
		prev_parent_bp = parent_bp;
		//sync_printf("DEBUG 3\n");
		parent_bp = *parent_bp;
		//sync_printf("DEBUG 4\n");
	}
	//sync_printf("DEBUG 5\n");
	//sync_printf("Updating initial stack frame\n");
	//sync_printf("DEBUG 6\n");
	//sync_printf("prev_parent_bp = %X\n", prev_parent_bp);
	//sync_printf("%X <- %X\n", (uint32 *)((uint32)prev_parent_bp + stack_offset - 4), (uint32)(*(uint32 *)((uint32)prev_parent_bp - 4)) + stack_offset);
	*(uint32 *)((uint32)prev_parent_bp + stack_offset - 4)
		= (uint32)(*(uint32 *)((uint32)prev_parent_bp - 4)) + stack_offset;
	//sync_printf("%X <- %X\n", (uint32 *)((uint32)prev_parent_bp + stack_offset - 28), (uint32)(*(uint32 *)((uint32)prev_parent_bp - 28)) + stack_offset);
	*(uint32 *)((uint32)prev_parent_bp + stack_offset - 28)
		= (uint32)(*(uint32 *)((uint32)prev_parent_bp - 28)) + stack_offset;
	//sync_printf("%X <- %X\n", (uint32 *)((uint32)prev_parent_bp + stack_offset - 32), (uint32)(*(uint32 *)((uint32)prev_parent_bp - 32)) + stack_offset);
	*(uint32 *)((uint32)prev_parent_bp + stack_offset - 32)
		= (uint32)(*(uint32 *)((uint32)prev_parent_bp - 32)) + stack_offset;

	/* The following entries on the stack must match what ctxsw	*/
	/*   expects a saved process state to contain: ret address,	*/
	/*   ebp, interrupt mask, flags, registers, and an old SP	*/

	//sync_printf("Setting up context switch\n");

	asm volatile("mov $., %0" : "=r"(child_eip) );

	if (parent_pid == getpid()) {
		*--saddr = child_eip;	/* Make the stack look like it's*/
						/*   half-way through a call to	*/
						/*   ctxsw that "returns" to the*/
						/*   new process		*/
		*--saddr = (long)((uint32)(parent_bp_register) + stack_offset);		/* This will be register ebp	*/
					/*   for process exit		*/
		savsp = (uint32) saddr;		/* Start of frame for ctxsw	*/
		*--saddr = 0x00000200;		/* New process runs with	*/
						/*   interrupts enabled		*/

		/* Basically, the following emulates an x86 "pushal" instruction*/

		*--saddr = 0;			/* %eax */
		*--saddr = 0;			/* %ecx */
		*--saddr = 0;			/* %edx */
		*--saddr = 0;			/* %ebx */
		*--saddr = 0;			/* %esp; value filled in below	*/
		pushsp = saddr;			/* Remember this location	*/
		*--saddr = savsp;		/* %ebp (while finishing ctxsw)	*/
		*--saddr =  0;			/* %esi */
		*--saddr =  0;			/* %edi */
		*pushsp = (unsigned long) (prptr->prstkptr = (char *)saddr);

		//xsh_ps(1);

		// sync_printf("\n--- PARENT STACK ---\n");
		// stacktrace(getpid());
		// sync_printf("\n--- CHILD STACK ---\n");
		// stacktrace(pid);
		// sync_printf("\n--- END OF STACKS ---\n");

		//sync_printf("Add child to ready list\n");

		insert(pid, readylist, prptr->prprio);  // insert child into the ready list
		restore(mask);

		//sync_printf("Return from fork\n");

		return pid;
	}
	//child_start:
		//sync_printf("I'm Child: NPROC = %d\n", NPROC);
		return NPROC;
}