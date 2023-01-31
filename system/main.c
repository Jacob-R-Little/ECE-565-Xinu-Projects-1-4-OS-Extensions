#include <xinu.h>
#include <stdio.h>

void timed_execution(uint32 runtime){
	    while(proctab[currpid].runtime<runtime);
}

int main() {
	uint32 i;
	pid32 prA, prB;

	kprintf("P1 Runtime,P1 Turnaroundtime,P2 Runtime,P2 Turnaroundtime\n");

	for (i=1; i<51; i++) {
		prA = create_user_process(timed_execution, 1024, "timed_execution", 1, 100*i);
		prB = create_user_process(timed_execution, 1024, "timed_execution", 1, 100*i);

		set_tickets(prA, 50);
		set_tickets(prB, 50);
		
		resume(prA);
		resume(prB);

		receive();	
		receive();	

		sleepms(50); // wait for user processes to terminate	

		kprintf("%d,%d,", proctab[prA].runtime, proctab[prA].turnaroundtime);
		kprintf("%d,%d\n", proctab[prB].runtime, proctab[prB].turnaroundtime);
		}

	return OK;
}
