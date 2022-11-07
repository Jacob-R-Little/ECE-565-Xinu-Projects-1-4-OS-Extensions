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

process p_double_run_lock(pi_lock_t *l1, pi_lock_t *l2){
	pi_lock(l1);
    run_for_ms(200);
	pi_lock(l2);
	run_for_ms(1000);
	pi_unlock(l2);
    run_for_ms(200);
	pi_unlock(l1);		
	return OK;
}

process p_double_lock(pi_lock_t *l1, pi_lock_t *l2){
	pi_lock(l1);
	pi_lock(l2);
	run_for_ms(1000);
	pi_unlock(l2);
	pi_unlock(l1);		
	return OK;
}
	
process p_lock(pi_lock_t *l){
	pi_lock(l);
	run_for_ms(1000);
	pi_unlock(l);		
	return OK;
}

process p_endless(){
	while(TRUE) {
		xsh_ps(1);
		run_for_ms(3000);
	}
	return OK;
}
	
process	main(void)
{
	
	pi_lock_t 	m1, m2, m3;  		
	pid32		pid1, pid2, pid3, pid4, pid5;
	uint32 		timestamp;

	pi_initlock(&m1);
	pi_initlock(&m2);
	pi_initlock(&m3);

	kprintf("\n\n=========== TEST 1: Priority Inversion with 2 threads / 1 mutex  ===============\n\n");

        pid1 = create((void *)p_lock, INITSTK, 1,"nthreads", 1, &m1);
		pid2 = create((void *)p_lock, INITSTK, 2,"nthreads", 1, &m1);

        timestamp = get_timestamp();

        resume(pid1);
        sleepms(100);
        resume(pid2);


        receive();
        receive();


        kprintf("Time = %d ms\n", get_timestamp()-timestamp);

	kprintf("\n\n=========== TEST 2: Priority Inversion with 3 threads / 2 mutexes ===============\n\n");

        pid1 = create((void *)p_lock, INITSTK, 1,"nthreads", 1, &m1);
        pid2 = create((void *)p_double_lock, INITSTK, 2,"nthreads", 2, &m2, &m1);
		pid3 = create((void *)p_lock, INITSTK, 3,"nthreads", 1, &m2);

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
        
	kprintf("\n\n=========== TEST 3: Priority Inversion with 4 threads / 3 mutexes  ===============\n\n");

        pid1 = create((void *)p_lock, INITSTK, 1,"nthreads", 1, &m1);
        pid2 = create((void *)p_double_lock, INITSTK, 2,"nthreads", 2, &m2, &m1);
		pid3 = create((void *)p_double_lock, INITSTK, 3,"nthreads", 2, &m3, &m2);
		pid4 = create((void *)p_lock, INITSTK, 4,"nthreads", 1, &m3);
		// pid5 = create((void *)p_endless, INITSTK, 3,"nthreads", 0);

        timestamp = get_timestamp();

        resume(pid1);
        sleepms(100);
        resume(pid2);
		sleepms(100);
        resume(pid3);
		sleepms(100);
        resume(pid4);
		// sleepms(100);
        // resume(pid5);

        receive();
        receive();
		receive();
        receive();

        kprintf("Time = %d ms\n", get_timestamp()-timestamp);

	kprintf("\n\n=========== TEST 4: Priority Inversion with 3 threads / 2 mutexes both waiting on first process ===============\n\n");

        pid1 = create((void *)p_double_run_lock, INITSTK, 1,"nthreads", 2, &m1, &m2);
        pid2 = create((void *)p_lock, INITSTK, 2,"nthreads", 2, &m1);
		pid3 = create((void *)p_lock, INITSTK, 3,"nthreads", 1, &m2);

        timestamp = get_timestamp();

        resume(pid1);
        sleepms(100);
        resume(pid2);
		sleepms(200);
        resume(pid3);

        receive();
        receive();
		receive();

        kprintf("Time = %d ms\n", get_timestamp()-timestamp);

	return OK;
}
