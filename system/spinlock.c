#include "xinu.h"

// sl_lock_t sl_locks[NSPINLOCKS];

syscall sl_initlock(sl_lock_t *l) {
    static uint32 lock_num = 0;
    if (lock_num == NSPINLOCKS) return SYSERR;
    l->flag = 0;
    l->owner = 0;
    lock_num++;
    return OK;
}

syscall sl_lock(sl_lock_t *l) {
    while (test_and_set(&l->flag, 1));
    l->owner = currpid;
    return OK;
}

syscall sl_unlock(sl_lock_t *l) {
    if (l->owner != currpid) {
        return SYSERR;
    }
    l->flag = 0;
    return OK;
}