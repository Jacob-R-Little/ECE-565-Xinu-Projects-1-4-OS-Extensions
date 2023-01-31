/* ready.c - ready */

#include <xinu.h>

qid16	readylist;			/* Index of ready list		*/
qid16	HPQ;				/* Index of highest priority queue list		*/
qid16	MPQ;				/* Index of medium priority queue list		*/
qid16	LPQ;				/* Index of lowest priority queue list		*/

/*------------------------------------------------------------------------
 *  ready  -  Make a process eligible for CPU service
 *------------------------------------------------------------------------
 */

syscall print_ready_list()
{
	intmask mask = disable();	/* Interrupt mask		*/
	qid16	next = firstid(readylist);
	qid16	tail = queuetail(readylist);
	

	kprintf("\n\n---READYLIST---\n");

	while(next != tail) {
		kprintf("%d\n", (uint32)next);
		next = queuetab[next].qnext;
	}

	kprintf("---------------\n\n");

	restore(mask);

	return OK;
}

status	ready(
	  pid32		pid		/* ID of process to make ready	*/
	)
{
	register struct procent *prptr;

	if (isbadpid(pid)) {
		return SYSERR;
	}

	/* Set process state to indicate ready and add to ready list */

	prptr = &proctab[pid];
	prptr->prstate = PR_READY;
	if (prptr->user_process == USER) {
		MLFQ_insert(pid);
	}
	else {
		insert(pid, readylist, prptr->prprio);
	}

	resched();

	return OK;
}
