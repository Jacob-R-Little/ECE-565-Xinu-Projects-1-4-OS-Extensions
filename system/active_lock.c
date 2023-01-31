#include "xinu.h"

syscall print_deadlock(pid32 table[]) {
    uint32  i, j, min_index;
    pid32   min;

    kprintf("deadlock_detected=");

    for (i=0; table[i]; i++) {
        min = NPROC;
        for (j=0; table[j]; j++) {
            if (table[j] < min) {
                min = table[j];
                min_index = j;
            }
        }
        table[min_index] = NPROC;
        if (i) kprintf("-");
        kprintf("P%d", min);
    }

    kprintf("\n");
    
    return OK;
}

syscall detect_deadlock(pid32 pid, al_lock_t *l) {
    uint32  i, j;
    pid32 deadlock_table[NALOCKS] = {0};

    // try to find circular dependencies
    for (i=0; TRUE; i++) {
        // if a process is already deadlocked, no need to look further
        if (proctab[pid].deadlock) return OK;

        // save PIDs in an array in order of traversal
        deadlock_table[i] = pid;

        // if the owner of the lock is equal to the current process
        // a circular dependency has been found
        if (l->owner == currpid) {
            // declare all processes in circle as deadlocked
            for (j=0; deadlock_table[j]; j++)
                proctab[deadlock_table[j]].deadlock = TRUE;
                
            print_deadlock(deadlock_table);
            return OK;
        }
        
        // if the owner of the current lock is also waiting on a lock
        // continue checking for circlular dependencies on that lock
        if (proctab[l->owner].prstate == PR_LOCK) {
            pid = l->owner;
            l = proctab[l->owner].lock;
            continue;
        }

        // otherwise no deadlock detected
        return OK;
    }
}

syscall	al_park(al_lock_t *l) {
	intmask mask = disable();

    if (proctab[currpid].parkfl) {
        proctab[currpid].prstate = PR_LOCK;
        proctab[currpid].lock = l;
        detect_deadlock(currpid, l);
	    resched();
    }
	
    restore(mask);
	return OK;
}

syscall al_unpark(al_lock_t *l) {
    intmask mask = disable();

    pid32 pid = dequeue(l->q);
    l->owner = pid;
    proctab[pid].parkfl = 0;
    proctab[pid].prstate = PR_READY;
    proctab[pid].lock = NULL;

    insert(pid, readylist, proctab[pid].prprio);

    restore(mask);
    return OK;
}

syscall al_setpark() {
    intmask mask = disable();

    proctab[currpid].parkfl = 1;

    restore(mask);
    return OK;
}

syscall al_initlock(al_lock_t *l) {
    static uint32 lock_num = 0;

    if (lock_num == NALOCKS) return SYSERR;

    l->owner = 0;
    l->flag = 0;
    l->guard = 0;
    l->q = newqueue();

    lock_num++;
    return OK;
}

syscall al_lock(al_lock_t *l) {
    while (test_and_set(&l->guard, 1))
        sleepms(QUANTUM);

    if (l->flag == 0) {
        l->owner = currpid;
        l->flag = 1;
        l->guard = 0;
    }
    else {
        enqueue(currpid, l->q);
        al_setpark();
        l->guard = 0;
        al_park(l);
    }
    
    return OK;
}

syscall al_unlock(al_lock_t *l) {
    while (test_and_set(&l->guard, 1))
        sleepms(QUANTUM);

    if (l->owner != currpid) {
        return SYSERR;
    }

    if (isempty(l->q)) {
        l->flag = 0;
    }
    else {
        al_unpark(l);
    }

    l->guard = 0;
    return OK;
}

bool8 al_trylock(al_lock_t *l) {
    while (test_and_set(&l->guard, 1))
        sleepms(QUANTUM);

    if (l->flag == 0) {
        l->owner = currpid;
        l->flag = 1;
        l->guard = 0;
        return TRUE;
    }

    l->guard = 0;
    return FALSE;
}