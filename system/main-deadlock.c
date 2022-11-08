/*  main.c  - main */

#include <xinu.h>
#include <stdlib.h>

uint32 x[6] = {0};

syscall sync_printf(char *fmt, ...)
{
        intmask mask = disable();
        void *arg = __builtin_apply_args();
        __builtin_apply((void*)kprintf, arg, 100);
        restore(mask);
        return OK;
}

void run_for_ms(uint32 time){
	uint32 start = proctab[currpid].runtime;
	while ((proctab[currpid].runtime-start) < time);
}

process p2(al_lock_t *l1, al_lock_t *l2, uint32 index1, uint32 index2){
	al_lock(l1);

	run_for_ms(500);
	x[index1]++;

	al_lock(l2);

	run_for_ms(500);
	x[index1]++;
	x[index2]++;

	al_unlock(l1);

	run_for_ms(500);

	x[index2]++;

	al_unlock(l2);		
	return OK;
}

process p2_try(al_lock_t *l1, al_lock_t *l2, uint32 index1, uint32 index2){
	while (TRUE) {
		if (al_trylock(l1) == FALSE) {
			sleepms(10 * (rand() % 20));	// wait between 10 - 200 ms
			continue; 		// try to acquire the lock again
		}

		run_for_ms(500);
		x[index1]++;

		if (al_trylock(l2) == FALSE) {
			run_for_ms(100);	// theoretical cleenup
			x[index1]--;
			al_unlock(l1);	// release previously held lock
			sleepms(10 * (rand() % 20));	// wait between 10 - 200 ms
			continue;		// restart execution
		}

		run_for_ms(500);
		x[index1]++;
		x[index2]++;

		al_unlock(l1);

		run_for_ms(500);
		x[index2]++;

		al_unlock(l2);
		return OK;
	}
}
	
process	main(void)
{	
	pid32 pid[6];		// threads 
	al_lock_t mutex[6];		// mutexes
	uint32 i;
	bool8 try_passed = TRUE;
	
	/* initialize al_locks */
	for (i=0; i<6; i++) al_initlock(&mutex[i]);

	kprintf("\n\n=========== TEST 1: Deadlock on 3 threads  ===================\n\n");

		for (i=0; i<3; i++)
			pid[i] = create((void *)p2, INITSTK, 5, "p2", 4, &mutex[i], &mutex[(i + 2) % 3], i, (i + 2) % 3);

		kprintf("Deadlock expected:\n\n");

		for (i=0; i<3; i++) {
			resume(pid[i]);
			sleepms(100);
		}

		while (proctab[pid[0]].deadlock == FALSE) sleepms(1000);

		kprintf("\nTEST PASSED\n");

	kprintf("\n\n=========== TEST 2: Deadlock on 3 threads circumvented by trylock  ===================\n\n");

		for (i=3; i<6; i++)
			pid[i] = create((void *)p2_try, INITSTK, 5, "p2", 4, &mutex[i], &mutex[(i + 2) % 3 + 3], i, (i + 2) % 3 + 3);

		kprintf("This might take a bit due to randomness in time to prevent unwanted phase behavior\n\n");

		for (i=3; i<6; i++) {
			resume(pid[i]);
			sleepms(100);
		}

		for (i=0; i<3; i++) receive();

		kprintf("All Processes Completed, Deadlock Avoided!\n\n");

		kprintf("All x values should be equal to 4:\n");

		for (i=3; i<6; i++) kprintf("x[%d] = %d\n", i, x[i]);

		for (i=3; i<6; i++)
			if (x[i] != 4) try_passed = FALSE;

		if (try_passed) kprintf("TEST PASSED\n");
		else kprintf("TEST FAILED\n");


	return OK;
}
