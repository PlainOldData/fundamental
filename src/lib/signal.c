#include <lib/signal.h>
#include <stdlib.h>


#if defined(__linux__) || defined(__APPLE__)
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#elif defined(_WIN32)
#include <windows.h>
#endif


/* ------------------------------------------------------- [ Signal Type ] -- */


struct lib_signal_internal
{
        #if defined( _WIN32 )

        CRITICAL_SECTION mutex;
        CONDITION_VARIABLE condition;
        int value;

        #elif defined( __linux__ ) || defined( __APPLE__ )

        pthread_mutex_t mutex;
        pthread_cond_t condition;
        int value;

        #endif
};


/* --------------------------------------------------- [ Signal Lifetime ] -- */


lib_signal
lib_signal_create()
{
        struct lib_signal_internal *signal = malloc(sizeof(*signal));

        #if defined(_WIN32)

        InitializeCriticalSectionAndSpinCount(&signal->mutex, 32);
        InitializeConditionVariable(&signal->condition);
        signal->value = 0;

        #elif defined( __linux__ ) || defined( __APPLE__ )

        pthread_mutex_init(&signal->mutex, NULL);
        pthread_cond_init(&signal->condition, NULL);
        signal->value = 0;

        #endif

        return (lib_signal)signal;
}


void
lib_signal_destroy(lib_signal s)
{
        struct lib_signal_internal *signal = (struct lib_signal_internal*)s;

        #if defined(_WIN32)

        DeleteCriticalSection(&signal->mutex);

        #elif defined( __linux__ ) || defined( __APPLE__ )

        pthread_mutex_destroy(&signal->mutex);
        pthread_cond_destroy(&signal->condition);

        #endif

        free(signal);
}


/* ---------------------------------------------------- [ Signal Actions ] -- */


void
lib_signal_raise(lib_signal s)
{
        struct lib_signal_internal *signal = (struct lib_signal_internal*)s;

        #if defined(_WIN32)

        EnterCriticalSection(&signal->mutex);
        signal->value = 1;
        LeaveCriticalSection(&signal->mutex);
        WakeConditionVariable(&signal->condition);

        #elif defined( __linux__ ) || defined( __APPLE__ )

        pthread_mutex_lock(&signal->mutex);
        signal->value = 1;
        pthread_mutex_unlock(&signal->mutex);
        pthread_cond_signal(&signal->condition);

        #endif
}


int
lib_signal_wait(lib_signal s, int timeout_ms)
{
        struct lib_signal_internal *signal = (struct lib_signal_internal*)s;

        #if defined( _WIN32 )

        int timed_out = 0;
        EnterCriticalSection(&signal->mutex);
        while (signal->value == 0)
        {
          int res = SleepConditionVariableCS(&signal->condition, &signal->mutex, timeout_ms < 0 ? INFINITE : timeout_ms);
          if (!res && GetLastError() == ERROR_TIMEOUT) { timed_out = 1; break; }
        }
        if (!timed_out) signal->value = 0;
        LeaveCriticalSection(&signal->mutex);
        return !timed_out;


        #elif defined( __linux__ ) || defined( __APPLE__ )

        struct timespec ts;
        if (timeout_ms >= 0)
        {
          struct timeval tv;
          gettimeofday(&tv, NULL);
          ts.tv_sec = time(NULL) + timeout_ms / 1000;
          ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (timeout_ms % 1000);
          ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
          ts.tv_nsec %= (1000 * 1000 * 1000);
        }

        int timed_out = 0;
        pthread_mutex_lock(&signal->mutex);
        while (signal->value == 0)
        {
          if (timeout_ms < 0)
            pthread_cond_wait(&signal->condition, &signal->mutex);
          else if (pthread_cond_timedwait(&signal->condition, &signal->mutex, &ts) == ETIMEDOUT)
          {
            timed_out = 1;
            break;
          }

        }
        if (!timed_out) signal->value = 0;
        pthread_mutex_unlock(&signal->mutex);
        return !timed_out;

        #else
        #error Unknown platform.
        #endif
}
