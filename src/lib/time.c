#include <lib/time.h>


#if defined(__linux__) || defined(__APPLE__)
#include <sys/time.h>
#include <time.h>
#include <math.h>
#elif defined(_WIN32)
#include <windows.h>
#include <sys/types.h>
#include <sys/timeb.h>
#endif


unsigned long
lib_time_get_current_ms()
{
        #if defined(__linux__) || defined(__APPLE__)
        struct timeval tv;
        gettimeofday(&tv, 0);
        return (tv.tv_sec * 1000L) + (tv.tv_usec / 1000L);

        #elif defined(_WIN32)
        struct _timeb timebuffer;
        _ftime64_s(&timebuffer);
        return (unsigned long)((timebuffer.time * 1000L) + timebuffer.millitm);
        #endif
}


void
lib_sleep_ms(unsigned long ms)
{
        #if defined(__linux__) || defined(__APPLE__)
        struct timespec tim, tim2;
        tim.tv_sec = ms / 1000;
        tim.tv_nsec = (ms % 1000) * 1000000;
        nanosleep(&tim, &tim2);
        #elif defined(_WIN32)
        Sleep((DWORD)ms);
        #endif
}
