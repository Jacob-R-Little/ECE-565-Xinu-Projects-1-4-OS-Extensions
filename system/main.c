/*  main.c  - main */

#include <xinu.h>

void print_process_flag()
{	
	struct	procent	*prptr;		/* pointer to process		*/
	uint32 i;
	intmask mask = disable();
	
	kprintf("--------------------------------\n");

	for (i = 0; i < NPROC; i++) {
		prptr = &proctab[i];
		if (prptr->prstate == PR_FREE) {  /* skip unused slots	*/
			continue;
		}
		// if (prptr->user_process) {
		// 	kprintf("- P%d:: USER\n", i);
		// }
		// else {
		// 	kprintf("- P%d:: SYSTEM\n", i);
		// }
	}

	kprintf("--------------------------------\n");

	restore(mask);
}

void print_process_stats()
{	
	struct	procent	*prptr;		/* pointer to process		*/
	uint32 i;
	intmask mask = disable();
	
	kprintf("--------------------------------\n");

	for (i = 0; i < NPROC; i++) {
		prptr = &proctab[i];
		if (prptr->prstate == PR_FREE) {  /* skip unused slots	*/
			continue;
		}
		kprintf("P%d:\n", i);
		kprintf("Runtime: %d ms\n", prptr->runtime);
		kprintf("Turn around time: %d ms\n", prptr->turnaroundtime);
		kprintf("# of Context Switches: %d\n", prptr->num_ctxsw);
	}

	kprintf("--------------------------------\n");

	restore(mask);
}

process test1(){
	
	kprintf("HELLO! I am process %d\n", getpid());

	return OK;
}

process	main(void)
{
	uint32 i;
	uint32 time_capture = clktime;

	printf("\n\n");

	resume(create((void *)test1, 8192, 8, "test1", 0));
	resume(create((void *)test1, 8192, 9, "test2", 0));
	resume(create((void *)test1, 8192, 10, "test3", 0));

	xsh_ps(1);
	print_ready_list();
	print_process_stats();

	while (clktime < time_capture + 1);

	print_process_stats();

	while (clktime < time_capture + 2);

	print_process_stats();

	return OK;
}
