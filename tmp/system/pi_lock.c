#include "xinu.h"

syscall fix_readylist(pid32 pid) {
    qid16	next = firstid(readylist);
	qid16	tail = queuetail(readylist);

    while(next != tail) {
        if (next == pid) {
            getitem(pid);
            insert(pid, readylist, proctab[pid].prprio);
            return OK;
        }
		next = queuetab[next].qnext;
	}
    return OK;
}

syscall priority_inheritance(pi_lock_t *l) {
    struct procent *ptcurr = &proctab[currpid];
    struct procent *ptowner = &proctab[l->owner];
    if (ptcurr->prprio > ptowner->prprio) {
        while (ptcurr->pi_lock) {
            kprintf("priority_change=P%d::%d-%d\n", l->owner, ptowner->prprio, ptcurr->prprio);
            if (ptowner->origprio == 0)     // don't save owner priority if it has been saved before
                ptowner->origprio = ptowner->prprio;   // save owner priority
            ptowner->prprio = ptcurr->prprio;   // transfer priority

            // if in readylist, remove, then re-insert to fix order
            fix_readylist(l->owner);

            // check if owner is also waiting on a lock and needs it's priority boosted
            l = ptowner->pi_lock;
            ptcurr = ptowner;
            ptowner = &proctab[l->owner];
        }
    }
    return OK;
}

syscall priority_return(pi_lock_t *l) {
    struct procent *ptcurr = &proctab[currpid];
    uint32 i;
    pri16 ret_prio = ptcurr->origprio;

    // if priority has been boosted, return priority
    if (ptcurr->origprio) {

        // if this process is the owner of another lock (or multiple)
        // check to see if that lock should cause a priority boost
        // and if so, give it the highest priority found
        for (i=0; i<NPROC; i++) {
            if ((proctab[i].pi_lock->owner == currpid) && (proctab[i].pi_lock != l) && (proctab[i].prprio > ret_prio)) {
                ret_prio = proctab[i].prprio;
            }
        }
        
        kprintf("priority_change=P%d::%d-%d\n", currpid, ptcurr->prprio, ret_prio);
        ptcurr->prprio = ret_prio;

        // only set origprio to 0 if it was used above
        if (ret_prio == ptcurr->origprio) ptcurr->origprio = 0;
    }
    return OK;
}

syscall	pi_park(pi_lock_t *l) {
	intmask mask = disable();

    if (proctab[currpid].parkfl) {
        proctab[currpid].prstate = PR_LOCK;
        proctab[currpid].pi_lock = l;
        priority_inheritance(l);
	    resched();
    }
	
    restore(mask);
	return OK;
}

syscall pi_unpark(pi_lock_t *l) {
    intmask mask = disable();

    pid32 pid = dequeue(l->q);
    l->owner = pid;
    proctab[pid].parkfl = 0;
    proctab[pid].prstate = PR_READY;
    proctab[pid].pi_lock = NULL;
    priority_return(l);
    insert(pid, readylist, proctab[pid].prprio);
    l->guard = 0;

    restore(mask);
    return OK;
}

syscall pi_setpark() {
    intmask mask = disable();

    proctab[currpid].parkfl = 1;

    restore(mask);
    return OK;
}

syscall pi_initlock(pi_lock_t *l) {
    static uint32 lock_num = 0;

    if (lock_num == NPILOCKS) return SYSERR;

    l->owner = 0;
    l->flag = 0;
    l->guard = 0;
    l->q = newqueue();

    lock_num++;
    return OK;
}

syscall pi_lock(pi_lock_t *l) {
    while (test_and_set(&l->guard, 1))
        sleepms(QUANTUM);

    if (l->flag == 0) {
        l->owner = currpid;
        l->flag = 1;
        l->guard = 0;
    }
    else {
        enqueue(currpid, l->q);
        pi_setpark();
        l->guard = 0;
        pi_park(l);
    }
    
    return OK;
}

syscall pi_unlock(pi_lock_t *l) {
    while (test_and_set(&l->guard, 1))
        sleepms(QUANTUM);

    if (l->owner != currpid) {
        return SYSERR;
    }

    if (isempty(l->q)) {
        l->flag = 0;
        l->guard = 0;
    }
    else {
        pi_unpark(l);
    }

    return OK;
}