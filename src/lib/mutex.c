#include <lib/mutex.h>
#include <lib/fundamental.h>
#include <stdlib.h>


/* based off https://github.com/mattiasgustavsson/libs/blob/master/thread.h */


#if defined(__linux__) || defined(__APPLE__)
#include <pthread.h>
#elif defined(_WIN32)
#include <windows.h>
#endif


/* ---------------------------------------------------- [ Mutex Lifetime ] -- */


lib_mutex
lib_mutex_create()
{
        #if defined(__linux__) || defined(__APPLE__)
        pthread_mutex_t *m = malloc(sizeof(*m));
        pthread_mutex_init(m, NULL);

        return (lib_mutex)m;

        #elif defined(_WIN32)
        CRITICAL_SECTION *m = malloc(sizeof(*m));
        InitializeCriticalSectionAndSpinCount((CRITICAL_SECTION*)m, 32);

        return (lib_mutex)m;
        #endif
}


void
lib_mutex_destroy(lib_mutex *m)
{
        #if defined(__linux__) || defined(__APPLE__)
        pthread_mutex_destroy((pthread_mutex_t*) *m);
        free(*m);
        #elif defined(_WIN32)
        DeleteCriticalSection((CRITICAL_SECTION*)*m);
        free(*m);
        #endif
}


/* ----------------------------------------------------- [ Mutex Actions ] -- */


void
lib_mutex_lock(lib_mutex m)
{
        #if defined(__linux__) || defined(__APPLE__)
        pthread_mutex_lock((pthread_mutex_t*)m);
        #elif defined(_WIN32)
        EnterCriticalSection((CRITICAL_SECTION*)m);
        #endif
}


void
lib_mutex_unlock(lib_mutex m)
{
        #if defined(__linux__) || defined(__APPLE__)
        pthread_mutex_unlock((pthread_mutex_t*)m);
        #elif defined(_WIN32)
        LeaveCriticalSection((CRITICAL_SECTION*)m);
        #endif
}
