#include "xinu.h"


syscall	park() {
	intmask mask = disable();

    if (proctab[currpid].parkfl) {
        proctab[currpid].prstate = PR_LOCK;
	    resched();
    }
	
    restore(mask);
	return OK;
}

syscall unpark(lock_t *l) {
    intmask mask = disable();

    pid32 pid = dequeue(l->q);
    l->owner = pid;
    proctab[pid].parkfl = 0;
    proctab[pid].prstate = PR_READY;
    insert(pid, readylist, proctab[pid].prprio);

    restore(mask);
    return OK;
}

syscall setpark() {
    intmask mask = disable();

    proctab[currpid].parkfl = 1;

    restore(mask);
    return OK;
}

syscall initlock(lock_t *l) {
    static uint32 lock_num = 0;

    if (lock_num == NSPINLOCKS) return SYSERR;

    l->owner = 0;
    l->flag = 0;
    l->guard = 0;
    l->q = newqueue();

    lock_num++;
    return OK;
}

syscall lock(lock_t *l) {
    while (test_and_set(&l->guard, 1))
        sleepms(QUANTUM);

    if (l->flag == 0) {
        l->owner = currpid;
        l->flag = 1;
        l->guard = 0;
    }
    else {
        enqueue(currpid, l->q);
        setpark();
        l->guard = 0;
        park();
    }
    
    return OK;
}

syscall unlock(lock_t *l) {
    while (test_and_set(&l->guard, 1))
        sleepms(QUANTUM);

    if (l->owner != currpid) {
        return SYSERR;
    }

    if (isempty(l->q)) {
        l->flag = 0;
    }
    else {
        unpark(l);
    }

    l->guard = 0;
    return OK;
}