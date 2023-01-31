/*  main.c  - main */

#include <xinu.h>

syscall sync_printf(char *fmt, ...)
{
        intmask mask = disable();
        void *arg = __builtin_apply_args();
        __builtin_apply((void*)kprintf, arg, 100);
        restore(mask);
        return OK;
}

process increment(uint32 *x, uint32 n, lock_t *mutex){
	uint32 i, j;	
	for (i=0; i<n; i++){
		lock(mutex);
		(*x)+=1;
		for (j=0; j<1000; j++);
		yield();
		unlock(mutex);
	}
	return OK;
}

process nthreads(uint32 nt, uint32 *x, uint32 n, lock_t *mutex){
	pid32 pids[nt];
	int i;
	for (i=0; i < nt; i++){
		pids[i] = create((void *)increment, INITSTK, 1,"p", 3, x, n, mutex);
		if (pids[i]==SYSERR){
			kprintf("nthreads():: ERROR - could not create process");
			return SYSERR;
		}
	}
	for (i=0; i < nt; i++){
		if (resume(pids[i]) == SYSERR){
			kprintf("nthreads():: ERROR - could not resume process");
			return SYSERR;
		}
	}
	for (i=0; i < nt; i++) receive();
	return OK;
}

process	main(void)
{
	uint32 x;			// shared variable
	unsigned nt;			// number of threads cooperating
	unsigned value = 10000; 	// target value of variable

	lock_t mutex;  			// lock

	kprintf("\n\n=====       Testing the LOCK w/ sleep&guard         =====\n");

	// 10 threads
	kprintf("\n\n================= TEST 1 = 10 threads ===================\n");
	x = 0;	nt = 10;
 	initlock(&mutex); 
	resume(create((void *)nthreads, INITSTK, 1,"nthreads", 4, nt, &x, value/nt, &mutex));
	receive(); 
	sync_printf("%d threads, n=%d, target value=%d\n", nt, value, x);
	if (x==value) kprintf("TEST PASSED.\n"); else kprintf("TEST FAILED.\n");

	// 20 threads
        kprintf("\n\n================= TEST 2 = 20 threads ===================\n");
        x = 0;  nt = 20;
 	initlock(&mutex); 
        resume(create((void *)nthreads, INITSTK, 1,"nthreads", 4, nt, &x, value/nt, &mutex));
        receive();
	sync_printf("%d threads, n=%d, target value=%d\n", nt, value, x);
        if (x==value) kprintf("TEST PASSED.\n"); else kprintf("TEST FAILED.\n");

	// 50 threads
        kprintf("\n\n================= TEST 3 = 50 threads ===================\n");
        x = 0;  nt = 50;
 	initlock(&mutex); 
        resume(create((void *)nthreads, INITSTK, 1,"nthreads", 4, nt, &x, value/nt, &mutex));
        receive();
	sync_printf("%d threads, n=%d, target value=%d\n", nt, value, x);
        if (x==value) kprintf("TEST PASSED.\n"); else kprintf("TEST FAILED.\n");

	return OK;
}
