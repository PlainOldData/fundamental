#include <tyrant/tyrant.h>
#include <jobs/jobs.h>
#include <lib/spin_lock.h>
#include <lib/thread.h>
#include <lib/assert.h>
#include <ctx/context.h>
#include <thread_dispatch/thread_local_storage.h>
#include <thread_dispatch/thread_process.h> /* todo remove this */
#include <lib/alloc.h>
#include <lib/array.h>
#include <fiber/fiber.h>
#include <stdlib.h>


#ifndef ROA_JOB_DEBUG_OUTPUT
#define ROA_JOB_DEBUG_OUTPUT 0
#endif

#ifdef ROA_JOB_DEBUG_OUTPUT
#include <stdio.h>
#endif


uint64_t
tyrant_job_submit(
        tyrant_ctx_t ctx,
        struct tyrant_job_desc *desc,
        int count)
{
        /* find this thread */
        struct thread_local_storage *tls = 0;
        int th_index = job_internal_find_thread_index(ctx);
        {
                LIB_ASSERT(th_index > -1);
                tls = &ctx->tls[th_index];
        }

        /* queue jobs */
        uint64_t marker;
        {
                lib_spin_lock_aquire(&tls->job_lock);

                /* new batch */
                struct job_batch new_batch;
                new_batch.count = count;

                new_batch.counter = calloc(sizeof(*new_batch.counter), 1);

                lib_atomic_int_store(new_batch.counter, (int)count);

                uint32_t new_batch_id = ++tls->batch_counter;

                marker = LIB_PACK3232((uint32_t)th_index, new_batch_id);

                lib_array_push(tls->batches, new_batch);
                lib_array_push(tls->batch_ids, new_batch_id);

                if(LIB_IS_ENABLED(ROA_JOB_DEBUG_OUTPUT)) {
                        printf("th: %d new batch %d, counter %p \n", tls->th_index, (int)marker, new_batch.counter);
                }

                /* push work */
                unsigned j;

                for (j = 0; j < count; ++j) {
                        struct job_internal job;
                        job.desc = desc[j];
                        job.counter = new_batch.counter;

                        if(LIB_IS_ENABLED(ROA_JOB_DEBUG_OUTPUT)) {
                                printf("th: %d, pushed %p\n", tls->th_index, job.desc.func);
                        }

                        lib_array_push(tls->pending_jobs, job);
                }

                if (LIB_IS_ENABLED(ROA_JOB_DEBUG_OUTPUT)) {
                        unsigned pending_job_count = lib_array_size(tls->pending_jobs);
                        printf("Pending Jobs %d \n", pending_job_count);
                }

                lib_spin_lock_release(&tls->job_lock);
                lib_signal_raise(ctx->signal_new_work);
        }

        return marker;
}


void
tyrant_job_wait(
        tyrant_ctx_t ctx,
        uint64_t job_marker)
{
        int thread_id = (int)LIB_LOWER_32_BITS(job_marker);
        uint32_t wait_on_batch_id = LIB_UPPER_32_BITS(job_marker);

        /* todo: good to check but thread_id negates the need to find index */

        /* push onto blocked queue with marker */
        int th_index = job_internal_find_thread_index(ctx);
        LIB_ASSERT(th_index > -1);
        LIB_ASSERT(th_index == thread_id);

        struct thread_local_storage *tls = &ctx->tls[th_index];
        struct roa_fiber *this_fiber = tls->executing_fiber.worker_fiber;
        LIB_ASSERT(this_fiber != 0);

        /* lock fibers and push pending */
        {
                lib_spin_lock_aquire(&tls->fiber_lock);

                lib_array_push(tls->blocked_fibers, tls->executing_fiber);
                lib_array_push(tls->blocked_fiber_batch_ids, wait_on_batch_id);

                tls->executing_fiber.worker_fiber = 0;

                lib_spin_lock_release(&tls->fiber_lock);
        }

        /* fiber switching */
        {
                /* switch back to home fiber ... */

                roa_fiber_switch(this_fiber, tls->home_fiber);

                /* ... back from sleeping fiber */
        }
}
