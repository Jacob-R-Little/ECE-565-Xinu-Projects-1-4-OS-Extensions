/* resched.c - resched, resched_cntl */

#include <xinu.h>

//#define DEBUG_CTXSW
//#define DEBUG_MLFQ
//#define DEBUG_RESCHEDs

struct	defer	Defer;

uint32	boost_counter;
uint32	PQ_counter;
bool8	async_resched;

void print_MLFQ() {
	qid16	next = firstid(HPQ);
	qid16	tail = queuetail(HPQ);

	kprintf("HPQ: ");

	while(next != tail) {
		kprintf("%d, ", (uint32)next);
		next = queuetab[next].qnext;
	}

	kprintf("\nMPQ: ");

	next = firstid(MPQ);
	tail = queuetail(MPQ);

	while(next != tail) {
		kprintf("%d, ", (uint32)next);
		next = queuetab[next].qnext;
	}

	kprintf("\nLPQ: ");

	next = firstid(LPQ);
	tail = queuetail(LPQ);

	while(next != tail) {
		kprintf("%d, ", (uint32)next);
		next = queuetab[next].qnext;
	}

	kprintf("\n");
}

bool8 MLFQ_nonempty() {
	return nonempty(HPQ) || nonempty(MPQ) || nonempty(LPQ);
}

void MLFQ_insert(pid32 pid) {
	switch (proctab[pid].prprio) {
		case HPQPRIO:
			enqueue(pid, HPQ);
			break;
		case MPQPRIO:
			enqueue(pid, MPQ);
			break;
		case LPQPRIO:
			enqueue(pid, LPQ);
			break;
		default:
			break;
	}
}

pid32 mlfq(void) {

	pid32 pid, tail;
	struct procent *ptr, *oldptr;
	oldptr = &proctab[currpid];
	#ifdef DEBUG_MLFQ
		print_MLFQ();
	#endif

	if (async_resched) {
		PQ_counter = 0;
	}
	async_resched = TRUE;

	if (boost_counter >= PRIORITY_BOOST_PERIOD) {
		#ifdef DEBUG_MLFQ
			kprintf("----------PRIORITY BOOST----------\n");
		#endif
		boost_counter = 0;
		pid = firstid(HPQ);
		tail = queuetail(HPQ);
		while (pid != tail) {
			ptr = &proctab[pid];
			ptr->time_allotment = 0;
			pid = queuetab[pid].qnext;
		}
		while (nonempty(MPQ)) {
			pid = dequeue(MPQ);
			ptr = &proctab[pid];
			ptr->time_allotment = 0;
			ptr->prprio = HPQPRIO;
			enqueue(pid, HPQ);
		}
		while (nonempty(LPQ)) {
			pid = dequeue(LPQ);
			ptr = &proctab[pid];
			ptr->time_allotment = 0;
			ptr->prprio = HPQPRIO;
			enqueue(pid, HPQ);
		}
		pid = firstid(sleepq);
		tail = queuetail(sleepq);
		while (pid != tail) {
			ptr = &proctab[pid];
			if (ptr->user_process == USER) {
				ptr->prprio = HPQPRIO;
			}
			pid = queuetab[pid].qnext;
		}

		#ifdef DEBUG_MLFQ
			print_MLFQ();
		#endif
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
			enqueue(pid, MPQ);
		}
	}

	if (oldptr->prprio == MPQPRIO && (PQ_counter % 2)) {
		return getitem(currpid);
	}

	while (nonempty(MPQ)) {
		pid = dequeue(MPQ);
		ptr = &proctab[pid];
		if (ptr->time_allotment < TIME_ALLOTMENT) {
			return pid;
		}
		else {
			ptr->time_allotment = 0;
			ptr->prprio = LPQPRIO;
			enqueue(pid, LPQ);
		}
	}

	if (oldptr->prprio == LPQPRIO && (PQ_counter % 4)) {
		return getitem(currpid);
	}
	
	while (nonempty(LPQ)) {
		pid = dequeue(LPQ);
		return pid;
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
		if (ptold->user_process == USER) {	// old process is USER
			if (ptold->prprio > firstkey(readylist)) {
				#ifdef DEBUG_RESCHED
					kprintf("USER->USER\n");
				#endif
				MLFQ_insert(currpid);
				currpid = mlfq();
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
				MLFQ_insert(currpid);
				currpid = dequeue(readylist);
			}
			
			ptold->prstate = PR_READY;
		}
		else {	// old process is SYSTEM
			if (ptold->prprio > firstkey(readylist)) {
				if ((currpid == NULLPROC) && MLFQ_nonempty()) {
					#ifdef DEBUG_RESCHED
						kprintf("SYSTEM->USER\n");
					#endif
					/* Put Null process back in the ready list	*/
					ptold->prstate = PR_READY;
					insert(currpid, readylist, ptold->prprio);
					currpid = mlfq();
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

				if ((firstid(readylist) == NULLPROC) && MLFQ_nonempty())
					currpid = mlfq();
				else
					currpid = dequeue(readylist);
			}
		}
	}
	else {	/* Process is no longer eligible */
		if ((firstid(readylist) == NULLPROC) && MLFQ_nonempty()) {
			#ifdef DEBUG_RESCHED
				kprintf("KILL->USER\n");
			#endif
			currpid = mlfq();
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
