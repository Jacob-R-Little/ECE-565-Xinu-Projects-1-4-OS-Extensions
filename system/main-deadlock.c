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
	while ((proctab[currpid].runtime-start) < time);
}

process p2(al_lock_t *l1, al_lock_t *l2){
	al_lock(l1);
	run_for_ms(1000);
	al_lock(l2);		
	run_for_ms(1000);
	al_unlock(l1);
	run_for_ms(1000);
	al_unlock(l2);		
	run_for_ms(1000);
	return OK;
}
	
process	main(void)
{	
	pid32 pid1, pid2, pid3;		// threads 
	al_lock_t m1, m2, m3;		// mutexes
	uint32 		timestamp;
	
	/* initialize al_locks */
	al_initlock(&m1);
	al_initlock(&m2);
	al_initlock(&m3);

	kprintf("\n\n=========== TEST 1: Deadlock on 3 threads  ===================\n\n");

		/* first deadlock: 2 threads */	
		pid1 = create((void *)p2, INITSTK, 5, "p2", 2, &m1, &m2);
		pid2 = create((void *)p2, INITSTK, 5, "p2", 2, &m3, &m2);
		pid3 = create((void *)p2, INITSTK, 5, "p2", 2, &m3, &m1);

		timestamp = get_timestamp();

		resume(pid1);
		sleepms(100);
		resume(pid2);
		sleepms(100);
		resume(pid3);

		receive();
		receive();
		receive();

		kprintf("Time = %d ms\n", get_timestamp()-timestamp);

	kprintf("\n\n=========== TEST 2: Deadlock on 3 threads circumvented by trylock  ===================\n\n");
	
		/* first deadlock: 2 threads */	
		pid1 = create((void *)p2, INITSTK, 5, "p2", 2, &m1, &m2);
		pid2 = create((void *)p2, INITSTK, 5, "p2", 2, &m[1], &mutex[0]);
		pid3 = create((void *)p2, INITSTK, 5, "p2", 2, &mutex[1], &mutex[0]);

		timestamp = get_timestamp();

		resume(pid1);
		sleepms(100);
		resume(pid2);
		sleepms(100);
		resume(pid3);

		receive();
		receive();
		receive();

		kprintf("Time = %d ms\n", get_timestamp()-timestamp);

	return OK;
}
