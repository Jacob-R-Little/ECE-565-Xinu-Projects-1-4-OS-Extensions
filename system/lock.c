#include "xinu.h"

syscall	park () {
	intmask mask = disable();

	// proctab[currpid].prstate = PR_SLEEP;
	// resched();
	// restore(mask);

	return OK;
}

void unpark () {
    intmask mask = disable();

    return OK;
}

void setpark() {
    intmask mask = disable();

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
    while (test_and_set(&l->guard, 1));

    if (l->flag == 0) {
        l->owner = currpid;
        l->flag = 1;
        l->guard = 0;
    }
    else {
        enqueue(l->q, currpid);
        l->guard = 0;
        park();
    }
    
    return OK;
}

syscall unlock(lock_t *l) {
    while (test_and_set(&l->guard, 1));

    if (l->owner != currpid) {
        return SYSERR;
    }

    if (isempty(l->q))
        l->flag = 0;
    else
        unpark(dequeue(l->q));

    l->guard = 0;
    return OK;
}