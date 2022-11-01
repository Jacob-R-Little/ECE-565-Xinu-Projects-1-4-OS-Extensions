#include "xinu.h"


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

syscall	park () {
	intmask mask = disable();

    if (proctab[currpid].parkfl) {
        proctab[currpid].prstate = PR_LOCK;
	    resched();
    }
	
    restore(mask);
	return OK;
}

syscall unpark (pid32 pid) {
    intmask mask = disable();

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

    print_queue(readylist);
    lock_printf("Expected NQENT: %d\n", NPROC + 4 + NSEM + NSEM + NLOCKS + NLOCKS);
    lock_printf("Real NQENT: %d\n", NQENT);

    lock_printf("Init Lock | ");
    l->owner = 0;
    l->flag = 0;
    l->guard = 0;
    l->q = newqueue();
    lock_printf("QID: %d\n", l->q);

    lock_num++;
    return OK;
}

syscall lock(lock_t *l) {
    while (test_and_set(&l->guard, 1))
        sleepms(QUANTUM);

    if (l->flag == 0) {
        //lock_printf("Lock Acquired | QID: %d | PID: %d\n", l->q, currpid);
        l->owner = currpid;
        l->flag = 1;
        l->guard = 0;
    }
    else {
        intmask mask = disable();
        //lock_printf("Parking | QID: %d | PID: %d\n", l->q, currpid);
        enqueue(currpid, l->q);
        //print_queue(l->q);
        setpark();
        l->guard = 0;
        park();
        restore(mask);
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
        //lock_printf("Lock Released | QID: %d | PID: %d\n", l->q, currpid);
        l->flag = 0;
    }
    else {
        intmask mask = disable();
        pid32 pid = dequeue(l->q);
        l->owner = pid;
        //lock_printf("Unparking | QID: %d | PID: %d\n", l->q, pid);
        unpark(pid);
        //print_queue(l->q);
        restore(mask);
    }

    l->guard = 0;
    return OK;
}