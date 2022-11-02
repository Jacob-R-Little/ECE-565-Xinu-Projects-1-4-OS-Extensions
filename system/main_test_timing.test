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

uint32 get_timestamp(){
	return ctr1000;
}

void run_for_ms(uint32 time){
	uint32 start = proctab[currpid].runtime;
	while (proctab[currpid].runtime-start < time);
}

process p_spinlock(sl_lock_t *l){
	uint32 i;
	for (i=0; i<5; i++){
		sl_lock(l);
		run_for_ms(1000);
		sl_unlock(l);		
	}
	return OK;
}
	
process p_lock(lock_t *l){
	uint32 i;
	for (i=0; i<5; i++){
		lock(l);
		run_for_ms(1000);
		unlock(l);		
	}
	return OK;
}
	
process	main(void)
{

	sl_lock_t	mutex_sl;  		
	lock_t 		mutex;  		
	pid32		pid1, pid2;
	uint32 		timestamp;

	kprintf("\n\n=========== TEST 1: spinlock & 2 threads  ===================\n\n");
 	sl_initlock(&mutex_sl); 
	
	pid1 = create((void *)p_spinlock, INITSTK, 1,"nthreads", 1, &mutex_sl);
	pid2 = create((void *)p_spinlock, INITSTK, 1,"nthreads", 1, &mutex_sl);

	timestamp = get_timestamp();
	
	resume(pid1);
	sleepms(500);
	resume(pid2);		

	receive();
	receive();

        kprintf("Time = %d ms\n", get_timestamp()-timestamp);
        
	kprintf("\n\n=========== TEST 2: lock w/sleep & 2 threads  ===============\n\n");
	initlock(&mutex);

        pid1 = create((void *)p_lock, INITSTK, 1,"nthreads", 1, &mutex);
        pid2 = create((void *)p_lock, INITSTK, 1,"nthreads", 1, &mutex);

        timestamp = get_timestamp();

        resume(pid1);
        sleepms(500);
        resume(pid2);

        receive();
        receive();

        kprintf("Time = %d ms\n", get_timestamp()-timestamp);

	return OK;
}
