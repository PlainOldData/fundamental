#ifndef JOBS_INCLDUED_7FA641F2_CAD6_4AD8_A406_E03E1FD4F2BC
#define JOBS_INCLDUED_7FA641F2_CAD6_4AD8_A406_E03E1FD4F2BC


#include <lib/fundamental.h>
#include <tyrant/tyrant.h>
#include <lib/atomic.h>


struct job_batch {
        int count;
        lib_atomic_int *counter;
};


struct job_internal {
        struct tyrant_job_desc desc;
        /* uint32_t batch_id; */
        lib_atomic_int *counter;
};


#endif /* inc guard */
