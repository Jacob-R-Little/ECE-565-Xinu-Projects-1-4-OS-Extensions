#include <xinu.h>
#include <stdio.h>

pid32 prA, prB, prC, prD;
uint32 a_cnt, b_cnt, c_cnt, d_cnt;

void compute(uint32 runtime, uint32 *value)
{   
        int i;
        while (proctab[currpid].runtime<runtime) {
            for (i = 0; i < 1000; i++)
                ;
            (*value)++;
        }
}

int main() {

	double total_cnt, a_percent, b_percent, c_percent, d_percent;
	
	a_cnt = b_cnt = c_cnt = d_cnt = 0;	
	
	kprintf("\n\n================= TESTCASE 1: 2 processes ======================\n");
	total_cnt=0;
	a_percent=0;
	b_percent=0;
	prA = create_user_process(compute, 1024, "process A", 2, 10000, &a_cnt);
	prB = create_user_process(compute, 1024, "process B", 2, 10000, &b_cnt);
	set_tickets(prA, 1000);
	set_tickets(prB, 1000);
	resume(prA);
	resume(prB);
	sleepms(4000);
	kill(prA);
	kill(prB);
	total_cnt = a_cnt + b_cnt ;
	a_percent = (double) a_cnt / total_cnt * 100;
	b_percent = (double) b_cnt / total_cnt * 100;
	kprintf("Test RESULT: A : B (%d : %d)\n", (int) a_percent, (int) b_percent);

	
	a_cnt = b_cnt = c_cnt = d_cnt = 0;	
	kprintf("\n\n================= TESTCASE 2: 2 processes ======================\n");
	total_cnt=0;
	a_percent=0;
	b_percent=0;
	prA = create_user_process(compute, 1024, "process A", 2, 10000, &a_cnt);
	prB = create_user_process(compute, 1024, "process B", 2, 10000, &b_cnt);
	set_tickets(prA, 1000);
	set_tickets(prB, 0);
	resume(prA);
	resume(prB);
	sleepms(4000);
	kill(prA);
	kill(prB);
	total_cnt = a_cnt + b_cnt ;
	a_percent = (double) a_cnt / total_cnt * 100;
	b_percent = (double) b_cnt / total_cnt * 100;
	kprintf("Test RESULT: A : B (%d : %d)\n", (int) a_percent, (int) b_percent);
 
	
	a_cnt = b_cnt = c_cnt = d_cnt = 0;	
	kprintf("\n\n================= TESTCASE 3: 2 processes ======================\n");
	total_cnt=0;
	a_percent=0;
	b_percent=0;
	prA = create_user_process(compute, 1024, "process A", 2, 10000, &a_cnt);
	prB = create_user_process(compute, 1024, "process B", 2, 1000, &b_cnt);
	set_tickets(prA, 1000);
	set_tickets(prB, 1000);
	resume(prA);
	resume(prB);
	sleepms(4000);
	kill(prA);
	kill(prB);
	total_cnt = a_cnt + b_cnt ;
	a_percent = (double) a_cnt / total_cnt * 100;
	b_percent = (double) b_cnt / total_cnt * 100;
	kprintf("Test RESULT: A : B (%d : %d)\n", (int) a_percent, (int) b_percent);

	
	a_cnt = b_cnt = c_cnt = d_cnt = 0;	
	kprintf("\n\n================= TESTCASE 4: 4 processes ======================\n");
	total_cnt=0;
	a_percent=0;
	b_percent=0;
	a_cnt=b_cnt=0;
	prA = create_user_process(compute, 1024, "process A", 2, 10000, &a_cnt);
	prB = create_user_process(compute, 1024, "process B", 2, 10000, &b_cnt);
	prC = create_user_process(compute, 1024, "process C", 2, 10000, &c_cnt);
	prD = create_user_process(compute, 1024, "process D", 2, 10000, &d_cnt);
	set_tickets(prA, 2000);
	set_tickets(prB, 1000);
	set_tickets(prC, 500);
	set_tickets(prD, 250);
	resume(prA);
	resume(prB);
	resume(prC);
	resume(prD);
	sleepms(8000);
	kill(prA);
	kill(prB);
	kill(prC);
	kill(prD);
	total_cnt = a_cnt + b_cnt + c_cnt + d_cnt;
	a_percent = (double) a_cnt / total_cnt * 100;
	b_percent = (double) b_cnt / total_cnt * 100;
	c_percent = (double) c_cnt / total_cnt * 100;
	d_percent = (double) d_cnt / total_cnt * 100;
	kprintf("Test RESULT: A : B : C : D (%d : %d : %d : %d)\n",
	        (int) a_percent, (int) b_percent, (int) c_percent, (int) d_percent);


	a_cnt = b_cnt = c_cnt = d_cnt = 0;	
	kprintf("\n\n================= TESTCASE 5: 4 processes ======================\n");
	total_cnt=0;
	a_percent=0;
	b_percent=0;
	a_cnt=b_cnt=0;
	prA = create_user_process(compute, 1024, "process A", 2, 10000, &a_cnt);
	prB = create_user_process(compute, 1024, "process B", 2, 10000, &b_cnt);
	set_tickets(prA, 1000);
	set_tickets(prB, 1000);
	resume(prA);
	resume(prB);
	sleepms(4000);
	prC = create_user_process(compute, 1024, "process C", 2, 10000, &c_cnt);
	prD = create_user_process(compute, 1024, "process D", 2, 10000, &d_cnt);
	set_tickets(prC, 1000);
	set_tickets(prD, 1000);
	resume(prC);
	resume(prD);
	sleepms(4000);
	kill(prC);
	kill(prD);
	total_cnt = a_cnt + b_cnt + c_cnt + d_cnt;
	a_percent = (double) a_cnt / total_cnt * 100;
	b_percent = (double) b_cnt / total_cnt * 100;
	c_percent = (double) c_cnt / total_cnt * 100;
	d_percent = (double) d_cnt / total_cnt * 100;
	kprintf("Test RESULT: A : B : C : D (%d : %d : %d : %d)\n",
	        (int) a_percent, (int) b_percent, (int) c_percent, (int) d_percent);

	return OK;
}
