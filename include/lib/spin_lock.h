#ifndef SPIN_LOCK_INCLUDED_4762A828_13CF_4464_8A84_B22858A52084
#define SPIN_LOCK_INCLUDED_4762A828_13CF_4464_8A84_B22858A52084


#include <lib/atomic.h>


#ifdef __cplusplus
extern "C" {
#endif


/* ------------------------------------------------- [ Spin Lock Actions ] -- */


void
lib_spin_lock_init(lib_atomic_int *lock);


void
lib_spin_lock_aquire(lib_atomic_int *lock);


int
lib_spin_lock_try_aquire(lib_atomic_int *lock);


void
lib_spin_lock_release(lib_atomic_int *lock);


#ifdef __cplusplus
} /* extern */
#endif


#endif /* inc guard */
