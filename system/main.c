#include <xinu.h>
#include <stdio.h>

void timed_execution(uint32 runtime){
	    while(proctab[currpid].runtime<runtime);
}

int main() {
	pid32 prA, prB, prC, prD;

	kprintf("\n");
	
	kprintf("=== TESTCASE 1::  CPU-intensive jobs =============================\n");

	prA = create_user_process(timed_execution, 1024, "timed_execution", 1, 10000);
	prB = create_user_process(timed_execution, 1024, "timed_execution", 1, 10000);
	prC = create_user_process(timed_execution, 1024, "timed_execution", 1, 10000);

	set_tickets(prA, 50);
	set_tickets(prB, 50);
	set_tickets(prC, 0);
	
	resume(prA);
	resume(prB);
	resume(prC);

	receive();	
	receive();	

	sleepms(50); // wait for user processes to terminate	

	kprintf("process %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prA, proctab[prA].runtime, proctab[prA].turnaroundtime, proctab[prA].num_ctxsw);
	kprintf("process %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prB, proctab[prB].runtime, proctab[prB].turnaroundtime, proctab[prB].num_ctxsw);
	kprintf("process %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prC, proctab[prC].runtime, proctab[prC].turnaroundtime, proctab[prC].num_ctxsw);

	set_tickets(prC,1);
	
	receive();	
   
	sleepms(20); // wait for user processes to terminate	
 
	kprintf("process %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prC, proctab[prC].runtime, proctab[prC].turnaroundtime, proctab[prC].num_ctxsw);

	prD = create_user_process(timed_execution, 1024, "timed_execution", 1, 10000);
	set_tickets(prD,1);
	resume(prD);
	
	receive();	
	
	sleepms(20); // wait for user processes to terminate	
	
	kprintf("process %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prD, proctab[prD].runtime, proctab[prD].turnaroundtime, proctab[prD].num_ctxsw);

	kprintf("==================================================================\n\n");

	kprintf("=== TESTCASE 2::  interactive jobs ===============================\n");

	prA = create_user_process(burst_execution, 1024, "burst_execution", 3, 10, 500, 500);
	prB = create_user_process(burst_execution, 1024, "burst_execution", 3, 10, 500, 500);
	prC = create_user_process(burst_execution, 1024, "burst_execution", 3, 10, 500, 500);

	set_tickets(prA, 80);
	set_tickets(prB, 10);
	set_tickets(prC, 10);
	
	resume(prA);
	resume(prB);
	resume(prC);

	receive();	
	receive();	
	receive();	

	sleepms(50); // wait for user processes to terminate	

	kprintf("process %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prA, proctab[prA].runtime, proctab[prA].turnaroundtime, proctab[prA].num_ctxsw);
	kprintf("process %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prB, proctab[prB].runtime, proctab[prB].turnaroundtime, proctab[prB].num_ctxsw);
	kprintf("process %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prC, proctab[prC].runtime, proctab[prC].turnaroundtime, proctab[prC].num_ctxsw);

	kprintf("==================================================================\n\n");

	return OK;
}
