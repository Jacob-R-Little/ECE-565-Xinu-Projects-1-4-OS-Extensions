/* lock.h */

#define NSPINLOCKS 20
#define NLOCKS 20
#define NALOCKS 20

typedef struct sl_lock_t {
    pid32   owner;
    uint32  flag;
} sl_lock_t;

typedef struct lock_t {
    pid32   owner;
    uint32  flag;
    uing32  guard;
    qid16   q;
} lock_t;

typedef struct al_lock_t {
    pid32   owner;
    uint32  flag;
} al_lock_t;

/* function prototypes */
extern uint32 test_and_set(uint32 *, uint32);

extern syscall sl_initlock(sl_lock_t *l);
extern syscall sl_lock(sl_lock_t *l);
extern syscall sl_unlock(sl_lock_t *l);

extern syscall initlock(lock_t *l);
extern syscall lock(lock_t *l);
extern syscall unlock(lock_t *l);

