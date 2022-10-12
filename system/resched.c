/* resched.c - resched, resched_cntl */

#include <xinu.h>
#include <stdlib.h>

//#define DEBUG_CTXSW
//#define DEBUG_RESCHED
//#define DEBUG_LOTTERY

struct	defer	Defer;

pid32	lottery(void)
{
	pid32	pid;
	uint32 	counter = 0;
	uint32 	winner;
	uint32	total_tickets = 0;
	qid16	next = firstid(lotterylist);
	qid16	tail = queuetail(lotterylist);

	if (oneid(lotterylist)) {
		pid = dequeue(lotterylist);
		if (proctab[pid].tickets) {
			return pid;
		}
		else {
			insert(pid, lotterylist, proctab[pid].tickets);
			return dequeue(readylist);
		}
	}

	while (next != tail) {	// count total # of tickets
		total_tickets += queuetab[next].qkey;
		next = queuetab[next].qnext;
	}

	next = firstid(lotterylist);
	winner = rand() % total_tickets;
	#ifdef DEBUG_LOTTERY
		kprintf("  Win: %d / %d | Count:", winner, total_tickets);
	#endif

	while (next != tail) {	// determine the winner
		counter += queuetab[next].qkey;
		#ifdef DEBUG_LOTTERY
			kprintf("%d, ", counter);
		#endif
		if (counter > winner) {
			#ifdef DEBUG_LOTTERY
				kprintf("| Proc: %d\n", next);
			#endif
			return getitem(next);
		}

		next = queuetab[next].qnext;
	}

	// this should not happen but it's good to include just in case
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
		if (ptold->user_process == USER) {	// old process is USER
			if (ptold->prprio > firstkey(readylist)) {
				#ifdef DEBUG_RESCHED
					kprintf("USER->USER\n");
				#endif
				insert(currpid, lotterylist, ptold->tickets);
				currpid = lottery();
				if (currpid == oldpid) {
					#ifdef DEBUG_RESCHED
						kprintf("(continue)\n");
					#endif
					return;
				}
			}
			else {
				#ifdef DEBUG_RESCHED
					kprintf("USER->SYSTEM\n");
				#endif
				insert(currpid, lotterylist, ptold->tickets);
				currpid = dequeue(readylist);
			}
			
			ptold->prstate = PR_READY;
		}
		else {	// old process is SYSTEM
			if (ptold->prprio > firstkey(readylist)) {
				if ((currpid == NULLPROC) && nonempty(lotterylist)) {
					#ifdef DEBUG_RESCHED
						kprintf("SYSTEM->USER\n");
					#endif
					/* Put Null process back in the ready list	*/
					ptold->prstate = PR_READY;
					insert(currpid, readylist, ptold->prprio);
					currpid = lottery();
				}
				else {
					#ifdef DEBUG_RESCHED
						kprintf("SYSTEM->SYSTEM (continue)\n");
					#endif
					return;
				}
			}
			else {
				#ifdef DEBUG_RESCHED
					kprintf("SYSTEM->SYSTEM\n");
				#endif
				/* Old process will no longer remain current */
				ptold->prstate = PR_READY;
				insert(currpid, readylist, ptold->prprio);

				if ((firstid(readylist) == NULLPROC) && nonempty(lotterylist))
					currpid = lottery();
				else
					currpid = dequeue(readylist);
			}
		}
	}
	else {	/* Process is no longer eligible */
		if ((firstid(readylist) == NULLPROC) && nonempty(lotterylist)) {
			#ifdef DEBUG_RESCHED
				kprintf("KILL->USER\n");
			#endif
			currpid = lottery();
		}
		else {
			#ifdef DEBUG_RESCHED
				kprintf("KILL->SYSTEM\n");
			#endif
			currpid = dequeue(readylist);
		}
	}

	/* Force context switch to highest priority ready process */

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
