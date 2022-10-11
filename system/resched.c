/* resched.c - resched, resched_cntl */

#include <xinu.h>

#define DEBUG_CTXSW

struct	defer	Defer;

uint32 boost_counter;

pid32 mlfq(void) {

	pid32 pid;
	struct procent *ptr;

	ptr = &proctab[currpid];
	if (ptr->user_process == USER) {
		switch (ptr->prprio) {
			case HPQPRIO:
				enqueue(currpid, HPQ);
				break;
			case MPQPRIO:
				enqueue(currpid, MPQ);
				break;
			case LPQPRIO:
				enqueue(currpid, LPQ);
				break;
			default
				break;
		}	
	}

	if (boost_counter >= PRIORITY_BOOST_PERIOD) {
		boost_counter = 0;
		while (nonempty(MPQ)) {
			pid = dequeue(MPQ)
			ptr = &proctab[pid];
			ptr->prprio = HPQPRIO;
			enqueue(dequeue(MPQ),HPQ);
		}
		while (nonempty(LPQ)) {
			enqueue(dequeue(LPQ),HPQ);
		}
	}

	while(nonempty(HPQ)) {
		pid = dequeue(HPQ);
		ptr = &proctab[pid];
		if (ptr->time_allotment < TIME_ALLOTMENT) {
			return pid;
		}
		else {
			ptr->time_allotment = 0;
			ptr->prprio = MPQPRIO;
		}
	}
	if (TRUE) {
			while (nonempty(MPQ)) {
			pid = dequeue(MPQ);
			ptr = &proctab[pid];
			if (ptr->time_allotment < TIME_ALLOTMENT) {
				return pid;
			}
			else {
				ptr->time_allotment = 0;
				ptr->prprio = LPQPRIO;
			}
		}
	}
	if (TRUE) {
		while (nonempty(LPQ)) {
			pid = dequeue(MPQ);
			return pid;
		}
	}

	// this should never happen
	return dequeue(readylist);
}

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/
	pid32	oldpid = currpid;

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */

	ptold = &proctab[currpid];

	if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
		if (ptold->prprio > firstkey(readylist)) {
			return;
		}

		/* Old process will no longer remain current */

		ptold->prstate = PR_READY;
		insert(currpid, readylist, ptold->prprio);
	}

	/* Force context switch to highest priority ready process */

	currpid = dequeue(readylist);
	ptnew = &proctab[currpid];
	ptnew->prstate = PR_CURR;
	ptnew->num_ctxsw++;
	preempt = QUANTUM;		/* Reset time slice for process	*/
	#ifdef DEBUG_CTXSW
		if (oldpid != currpid) {
			kprintf("ctxsw::%d-%d\n", oldpid, currpid);
		}
	#endif
	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);


	/* Old process returns here when resumed */

	return;
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	resched_cntl(		/* Assumes interrupts are disabled	*/
	  int32	defer		/* Either DEFER_START or DEFER_STOP	*/
	)
{
	switch (defer) {

	    case DEFER_START:	/* Handle a deferral request */

		if (Defer.ndefers++ == 0) {
			Defer.attempt = FALSE;
		}
		return OK;

	    case DEFER_STOP:	/* Handle end of deferral */
		if (Defer.ndefers <= 0) {
			return SYSERR;
		}
		if ( (--Defer.ndefers == 0) && Defer.attempt ) {
			resched();
		}
		return OK;

	    default:
		return SYSERR;
	}
}
