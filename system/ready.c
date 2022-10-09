/* ready.c - ready */

#include <xinu.h>

qid16	readylist;			/* Index of ready list		*/
qid16	lotterylist;		/* Index of lottery list		*/

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
		// if (prptr->tickets)
			insert(pid, lotterylist, prptr->tickets);
	}
	else
		insert(pid, readylist, prptr->prprio);

	resched();

	return OK;
}
