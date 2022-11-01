/* lock.h */

#define NSPINLOCKS 20
#define NLOCKS 20
#define NALOCKS 20

//typedef uint32  sl_lock_t;

typedef struct sl_lock_t {
    pid32   owner;
    uint32  flag;
} sl_lock_t;

typedef	uint32	lock_t;
typedef uint32  al_lock_t;



/* function prototypes */
extern uint32 test_and_set(uint32 *, uint32);

extern syscall sl_initlock(sl_lock_t *l);
extern syscall sl_lock(sl_lock_t *l);
extern syscall sl_unlock(sl_lock_t *l);

