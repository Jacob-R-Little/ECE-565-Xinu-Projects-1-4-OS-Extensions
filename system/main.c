/*  main.c  - main */

#include <xinu.h>
#include <stdarg.h>

//#define TESTCASE1
//#define TESTCASE2
//#define TESTCASE3

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

	for (i = 0; i < NPROC; i++) {
		prptr = &proctab[i];
		if (prptr->prstate == PR_FREE) {  /* skip unused slots	*/
			continue;
		}
		kprintf("- P%d:: ", i);
		if (i != 0) {
			for (j = 0; j < NPROC; j++) {
				prptr = &proctab[j];
				if ((uint32)(prptr->prparent) == i) {
					kprintf("%d ", j);
				}
			}
		}
		kprintf("\n");
	}
	restore(mask);
}

process test1(){
	
	pid32 pid;

	pid = create((void *)test2, 8192, 50, "test2", 1, 4);
	resume(pid);
	sync_ps();
	pid = create((void *)test2, 8192, 50, "test2", 1, 3);
	resume(pid);
	sync_ps();
	pid = create((void *)test2, 8192, 50, "test2", 1, 6);
	resume(pid);
	sync_ps();
	receive();

	return OK;
}

process test2(uint32 n){
	
	pid32 pid;

	for (;n > 0; n--) {
		pid = create((void *)test2, 8192, 50, "test2", 1, 0);
		resume(pid);
		sync_ps();
	}

	receive();
	
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
	sync_ps();
	sync_print_pr_tree();
	receive();
	sync_printf("\n[END-TESTCASE]\n");
		
	return OK;
    
}
