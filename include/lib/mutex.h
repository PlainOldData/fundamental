#ifndef MUTEX_INCLUDED_98AA2690_6EDF_4BDB_81EC_5B941FC8BC3F
#define MUTEX_INCLUDED_98AA2690_6EDF_4BDB_81EC_5B941FC8BC3F


#ifdef __cplusplus
extern "C" {
#endif


/* -------------------------------------------------------- [ Mutex Type ] -- */


typedef void* lib_mutex;


/* ---------------------------------------------------- [ Mutex Lifetime ] -- */


lib_mutex
lib_mutex_create();


void
lib_mutex_destroy(lib_mutex *m);


/* ----------------------------------------------------- [ Mutex Actions ] -- */


void
lib_mutex_lock(lib_mutex m);


void
lib_mutex_unlock(lib_mutex m);


#ifdef __cplusplus
} /* extern */
#endif


#endif /* inc guard */