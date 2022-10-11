#include <xinu.h>
#include <stdio.h>

void timed_execution(uint32 runtime){
	    while(proctab[currpid].runtime<runtime);
}

int main() {
	pid32 prA, prB, prC, prD;

	kprintf("\n");

	kprintf("QUANTUM=%d, TIME_ALLOTMENT=%d, PRIORITY_BOOST_PERIOD=%d\n\n", QUANTUM, TIME_ALLOTMENT, PRIORITY_BOOST_PERIOD);
	
	kprintf("=== TESTCASE 1::  CPU-intensive jobs =============================\n");

	prA = create_user_process(timed_execution, 1024, "cpu_1000", 1, 1000);
	prB = create_user_process(timed_execution, 1024, "cpu_1000", 1, 1000);

	resume(prA);
	resume(prB);

	receive();	
	receive();	

	sleepms(50); // wait for user processes to terminate	

	kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prA, proctab[prA].prname, proctab[prA].runtime, proctab[prA].turnaroundtime, proctab[prA].num_ctxsw);
	kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prB, proctab[prB].prname, proctab[prB].runtime, proctab[prB].turnaroundtime, proctab[prB].num_ctxsw);

	kprintf("==================================================================\n\n");

        kprintf("=== TESTCASE 2::  CPU-intensive jobs =============================\n");

        prA = create_user_process(timed_execution, 1024, "cpu_1000", 1, 1000);
        prB = create_user_process(timed_execution, 1024, "cpu_1000", 1, 1000);
        prC = create_user_process(timed_execution, 1024, "cpu_1000_new", 1, 1000);
        prD = create_user_process(timed_execution, 1024, "cpu_1000_del", 1, 1000);

        resume(prA);
        resume(prB);
        
	receive();
	receive();

	resume(prC);

	sleepms(500);

        resume(prD);

        receive();
        receive();

        sleepms(50); // wait for user processes to terminate    

        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prA, proctab[prA].prname, proctab[prA].runtime, proctab[prA].turnaroundtime, proctab[prA].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prB, proctab[prB].prname, proctab[prB].runtime, proctab[prB].turnaroundtime, proctab[prB].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prC, proctab[prC].prname, proctab[prC].runtime, proctab[prC].turnaroundtime, proctab[prC].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prD, proctab[prD].prname, proctab[prD].runtime, proctab[prD].turnaroundtime, proctab[prD].num_ctxsw);

        kprintf("==================================================================\n\n");

        kprintf("=== TESTCASE 3::  interactive jobs =============================\n");

        prA = create_user_process(burst_execution, 1024, "burst_100/5/5", 3, 100, 5, 5);
        prB = create_user_process(burst_execution, 1024, "burst_100/5/5", 3, 100, 5, 5);
        prC = create_user_process(burst_execution, 1024, "burst_100/5/5", 3, 100, 5, 5);
        prD = create_user_process(burst_execution, 1024, "burst_100/5/5", 3, 100, 5, 5);

        resume(prA);
        resume(prB);
        resume(prC);
        resume(prD);

        receive();
        receive();
        receive();
        receive();

        sleepms(50); // wait for user processes to terminate    

        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prA, proctab[prA].prname, proctab[prA].runtime, proctab[prA].turnaroundtime, proctab[prA].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prB, proctab[prB].prname, proctab[prB].runtime, proctab[prB].turnaroundtime, proctab[prB].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prC, proctab[prC].prname, proctab[prC].runtime, proctab[prC].turnaroundtime, proctab[prC].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prD, proctab[prD].prname, proctab[prD].runtime, proctab[prD].turnaroundtime, proctab[prD].num_ctxsw);

        kprintf("==================================================================\n\n");

        kprintf("=== TESTCASE 4: mixed jobs =============================\n");

        prA = create_user_process(timed_execution, 1024, "cpu_1000", 1, 1000);
        prB = create_user_process(timed_execution, 1024, "cpu_1000", 1, 1000);
        prC = create_user_process(burst_execution, 1024, "burst_100/5/5", 3, 100, 5, 5);
        prD = create_user_process(burst_execution, 1024, "burst_100/5/5", 3, 100, 5, 5);

        resume(prA);
        resume(prB);
        resume(prC);
        resume(prD);

        receive();
        receive();
        receive();
        receive();

        sleepms(50); // wait for user processes to terminate    

        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prA, proctab[prA].prname, proctab[prA].runtime, proctab[prA].turnaroundtime, proctab[prA].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prB, proctab[prB].prname, proctab[prB].runtime, proctab[prB].turnaroundtime, proctab[prB].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prC, proctab[prC].prname, proctab[prC].runtime, proctab[prC].turnaroundtime, proctab[prC].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prD, proctab[prD].prname, proctab[prD].runtime, proctab[prD].turnaroundtime, proctab[prD].num_ctxsw);

        kprintf("==================================================================\n\n");

        kprintf("=== TESTCASE 5::  mixed jobs =============================\n");

        prA = create_user_process(timed_execution, 1024, "cpu_1000", 1, 1000);
        prB = create_user_process(timed_execution, 1024, "cpu_1000", 1, 1000);
        prC = create_user_process(burst_execution, 1024, "burst_100/2/8", 3, 100, 2, 8);
        prD = create_user_process(burst_execution, 1024, "burst_100/2/8", 3, 100, 2, 8);

        resume(prA);
        resume(prB);
        resume(prC);
        resume(prD);

        receive();
        receive();
        receive();
        receive();

        sleepms(50); // wait for user processes to terminate    

        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prA, proctab[prA].prname, proctab[prA].runtime, proctab[prA].turnaroundtime, proctab[prA].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prB, proctab[prB].prname, proctab[prB].runtime, proctab[prB].turnaroundtime, proctab[prB].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prC, proctab[prC].prname, proctab[prC].runtime, proctab[prC].turnaroundtime, proctab[prC].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prD, proctab[prD].prname, proctab[prD].runtime, proctab[prD].turnaroundtime, proctab[prD].num_ctxsw);

        kprintf("==================================================================\n\n");

        kprintf("=== TESTCASE 6::  mixed jobs =============================\n");

        prA = create_user_process(timed_execution, 1024, "cpu_1000", 1, 1000);
        prB = create_user_process(burst_execution, 1024, "burst_100/1/9", 3, 100, 1, 9);
        prC = create_user_process(burst_execution, 1024, "burst_100/5/5", 3, 100, 5, 5);
        prD = create_user_process(burst_execution, 1024, "burst_20/40/10", 3, 20, 40, 10);

        resume(prA);
        resume(prB);
        resume(prC);
        resume(prD);

        receive();
        receive();
        receive();
        receive();

        sleepms(50); // wait for user processes to terminate    

        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prA, proctab[prA].prname, proctab[prA].runtime, proctab[prA].turnaroundtime, proctab[prA].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prB, proctab[prB].prname, proctab[prB].runtime, proctab[prB].turnaroundtime, proctab[prB].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prC, proctab[prC].prname, proctab[prC].runtime, proctab[prC].turnaroundtime, proctab[prC].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prD, proctab[prD].prname, proctab[prD].runtime, proctab[prD].turnaroundtime, proctab[prD].num_ctxsw);

        kprintf("==================================================================\n\n");

	return OK;

}
