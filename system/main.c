/*  main.c  - main */

#include <xinu.h>
#include <stdarg.h>

pid32 pid1, pid2, pid3;

process test1(void);
process test2(uint32 n);


uint32 sum(uint32 a, uint32 b){
	return (a+b);
}

void sync_printf(char *fmt, ...)
{
    	intmask mask = disable();
	void *arg = __builtin_apply_args();
	__builtin_apply((void*)kprintf, arg, 100);
	restore(mask);
}

void sync_ps()
{
    	intmask mask = disable();
	xsh_ps(1);
	restore(mask);
}

void sync_print_process_flag()
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
		if (prptr->user_process) {
			kprintf("- P%d:: USER\n", i);
		}
		else {
			kprintf("- P%d:: SYSTEM\n", i);
		}
	}

	kprintf("--------------------------------\n");

	restore(mask);
}

void sync_print_pr_tree()
{	
	struct	procent	*prptr;		/* pointer to process		*/
	uint32 i, j;

		intmask mask = disable();

	kprintf("--------------------------------\n");

	for (i = 0; i < NPROC; i++) {
		prptr = &proctab[i];
		if (prptr->prstate == PR_FREE) {  /* skip unused slots	*/
			continue;
		}
		kprintf("- P%d:: ", i);
		for (j = 0; j < NPROC; j++) {
			prptr = &proctab[j];
			if ((prptr->prstate != PR_FREE) && ((uint32)(prptr->prparent) == i)) {
				kprintf("%d ", j);
			}
		}
		kprintf("\n");
	}

	kprintf("--------------------------------\n");

	restore(mask);
}

process test1(){

	pid1 = create((void *)test2, 8192, 50, "test2", 1, 4);
	resume(pid1);

	pid2 = create((void *)test2, 8192, 50, "test2", 1, 3);
	resume(pid2);

	pid3 = create((void *)test2, 8192, 50, "test2", 1, 6);
	resume(pid3);

	sleep(10);

	return OK;
}

process test2(uint32 n){
	
	pid32 pid;

	for (;n > 0; n--) {
		pid = create((void *)test2, 8192, 50, "test2", 1, 0);
		resume(pid);
	}

	while (TRUE) {
		sleep(10);
	}
	
	return OK;
}

process test3(int a, int b, int *c, int *d){
	return OK;
}

process	main(void)
{

	pid32 pid;

	sync_printf("\n[CASCADING TERMINATION TESTCASE]\n");

	pid = create((void *)test1, 8192, 50, "test1", 0);
	resume(pid);

	sync_printf("\n[Process Flags (SYSTEM/USER)]\n");
	sync_print_process_flag();

	sync_printf("\n[Process Tree before killing]\n");
	sync_print_pr_tree();

	/* kill a process with no children */
	sync_printf("\n[Kill process %d]\n[This process HAS NO children]\n", pid3+1);
	kill(pid3+1);
	sync_print_pr_tree();

	/* kill a process with children */
	sync_printf("\n[Kill process %d]\n[This process HAS children]\n", pid2);
	kill(pid2);
	sync_print_pr_tree();

	/* kill a process with children and grandchildren*/
	sync_printf("\n[Kill process %d]\n[This process HAS children AND grandchildren]\n", pid);
	kill(pid);
	sync_print_pr_tree();

	sync_printf("\n[END-TESTCASE]\n");
		
	return OK;
    
}
