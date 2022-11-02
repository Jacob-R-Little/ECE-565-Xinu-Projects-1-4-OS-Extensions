#include "xinu.h"

qid16 al_lock_queues[NALOCKS];
pid32 deadlock_table[NPROC];
uint32 al_lock_num = 0;

syscall print_queue(qid16 q)
{
	intmask mask = disable();	/* Interrupt mask		*/
	//qid16	next = firstid(q);
    qid16	next = queuehead(q);
	qid16	tail = queuetail(q);
	

	kprintf("QID: %d | ", q);

	while(next != tail) {
		kprintf("%d, ", (uint32)next);
		next = queuetab[next].qnext;
	}

    kprintf("%d, ", (uint32)next);

	kprintf("\n");

	restore(mask);

	return OK;
}

syscall lock_printf(char *fmt, ...)
{
        intmask mask = disable();
        void *arg = __builtin_apply_args();
        __builtin_apply((void*)kprintf, arg, 100);
        restore(mask);
        return OK;
}

bool8 detect_deadlock(pid32 pid, qid16 q) {
    static uint32 depth = 0;
    uint32 i, j;
    qid16 search_q, next, tail;
    bool8 deadlock, in_deadlock_table;
    intmask mask = disable();

    deadlock_table[depth] = pid;

    kprintf("depth=%d | ", depth);
    for (i=0; deadlock_table[i]; i++) {
        kprintf("%d, ", deadlock_table[i]);
    }
    kprintf("\n", depth);

    for (i=0; i<al_lock_num; i++) {
        search_q = al_lock_queues[i];
        next = firstid(search_q);
	    tail = queuetail(search_q);
        if (search_q != q) {
            while(next != tail) {

                if (depth && (next == deadlock_table[0])) {
                    return TRUE;
                }

                // in_deadlock_table = FALSE;
                // for (j=1; deadlock_table[i]; j++) {
                //     if (next == deadlock_table[i]) {
                //         in_deadlock_table = TRUE;
                //         break;
                //     }
                // }

                // if (in_deadlock_table == FALSE) {
                    depth++;
                    if (depth == NALOCKS) {
                        depth--;
                        return FALSE; 
                    }
                    deadlock = detect_deadlock(next, search_q);
                    depth--;
                    if (deadlock) {
                        if (depth == 0) {
                            kprintf("Detected\n");
                        }
                        return TRUE;
                    }
                // }
                next = queuetab[next].qnext;
            }
        }
    }

    deadlock_table[depth] = 0;

    restore(mask);
    return FALSE;
}

syscall	al_park() {
	intmask mask = disable();

    if (proctab[currpid].parkfl) {
        proctab[currpid].prstate = PR_LOCK;
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
    intmask mask = disable();
    
    if (al_lock_num == NALOCKS) return SYSERR;

    l->owner = 0;
    l->flag = 0;
    l->guard = 0;
    l->q = newqueue();
    al_lock_queues[al_lock_num] = l->q;

    al_lock_num++;

    restore(mask);
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
        detect_deadlock(currpid, l->q);
        al_setpark();
        l->guard = 0;
        al_park();
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
    
    return FALSE;
}