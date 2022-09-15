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

void sync_print_pr_tree()
{	
		intmask mask = disable();
	struct	procent	*prptr;		/* pointer to process		*/
	uint32 i, j;

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
	restore(mask);
}

process test1(){

	pid1 = create((void *)test2, 8192, 50, "test2", 1, 4);
	(&proctab[pid1])->user_process = TRUE;
	sync_print_pr_tree();
	resume(pid1);
	pid2 = create((void *)test2, 8192, 50, "test2", 1, 3);
	(&proctab[pid2])->user_process = TRUE;
	sync_print_pr_tree();
	resume(pid2);
	pid3 = create((void *)test2, 8192, 50, "test2", 1, 6);
	(&proctab[pid3])->user_process = TRUE;
	sync_print_pr_tree();
	resume(pid3);
	sleep(10);

	return OK;
}

process test2(uint32 n){
	
	pid32 pid;

	for (;n > 0; n--) {
		pid = create((void *)test2, 8192, 50, "test2", 1, 0);
		(&proctab[pid])->user_process = TRUE;
		sync_print_pr_tree();
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
	(&proctab[pid])->user_process = TRUE;
	sync_print_pr_tree();
	resume(pid);

	/* kill a process with no children */
	kill(pid3+1);
	sync_print_pr_tree();
	sync_ps();

	/* kill a process with children */
	kill(pid2);
	sync_print_pr_tree();
	sync_ps();

	/* kill a process with children and grandchildren*/
	kill(pid);
	sync_print_pr_tree();
	sync_ps();

	sync_printf("\n[END-TESTCASE]\n");
		
	return OK;
    
}
