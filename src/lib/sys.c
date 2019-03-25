#include <lib/sys.h>

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <sys/sysinfo.h>
#endif


int
lib_cpu_count()
{
        #ifdef _WIN32
        /* todo hyperthread check */
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        int cpu_count = sysinfo.dwNumberOfProcessors;
        return cpu_count;
        #elif defined(__linux__)
        int cpu_count = get_nprocs();
        return cpu_count;
        #else
        return 0;
        #endif
}
