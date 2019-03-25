#include <lib/spin_lock.h>
#include <lib/atomic.h>
#include <lib/fundamental.h>


#define SPIN_LOCKED_ID 0xdeaf
#define SPIN_UNLOCKED_ID 0xfeed


void
lib_spin_lock_init(lib_atomic_int *lock)
{
        lib_atomic_int_store(lock, SPIN_UNLOCKED_ID);
}


void
lib_spin_lock_aquire(lib_atomic_int *lock)
{
        while (1) {
                int lock_id = lib_atomic_int_compare_and_swap(
                        lock,
                        SPIN_UNLOCKED_ID,
                        SPIN_LOCKED_ID);

                if (lock_id == SPIN_UNLOCKED_ID) {
                        break;
                }
        }
}


int
lib_spin_lock_try_aquire(lib_atomic_int *lock)
{
        int lock_id = lib_atomic_int_compare_and_swap(
                lock,
                SPIN_UNLOCKED_ID,
                SPIN_LOCKED_ID);

        if (lock_id == SPIN_UNLOCKED_ID) {
                return LIB_TRUE;
        }

        return LIB_FALSE;
}


void
lib_spin_lock_release(lib_atomic_int *lock)
{
        lib_atomic_int_store(lock, SPIN_UNLOCKED_ID);
}


#undef SPIN_LOCKED_ID
#undef SPIN_UNLOCKED_ID
