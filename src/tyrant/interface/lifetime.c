#include <tyrant/tyrant.h>
#include <lib/assert.h>
#include <lib/alloc.h>
#include <lib/sys.h>
#include <lib/log.h>
#include <lib/thread.h>
#include <lib/atomic.h>
#include <lib/spin_lock.h>
#include <lib/array.h>
#include <jobs/jobs.h>
#include <fiber/fiber.h>
#include <ctx/context.h>
#include <thread_dispatch/thread_local_storage.h>
#include <thread_dispatch/thread_process.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef _WIN32
#include <execinfo.h>
#include <dlfcn.h>
#endif


/* ------------------------------------------------------------ [ config ] -- */


#ifndef ROA_JOB_CPU_AFFINITY
#define ROA_JOB_CPU_AFFINITY 0
#endif


#ifndef ROA_JOB_DEBUG_OUTPUT
#define ROA_JOB_DEBUG_OUTPUT 0
#endif


#ifdef ROA_JOB_DEBUG_OUTPUT
#include <stdio.h>
#endif


/* ---------------------------------------------------------- [ lifetime ] -- */


void
tyrant_ctx_create(
        tyrant_ctx_t *ctx,
        struct tyrant_ctx_setup_desc *desc)
{
        /* param check */
        LIB_ASSERT(ctx);

        /* create new ctx */
        struct tyrant_ctx *new_ctx = calloc(sizeof(*new_ctx), 1);

        /* early bail */
        if(!new_ctx) {
                return;
        }

        int max_cores = 32;
        int free_cores = 1;

        if (desc != 0) {
                max_cores = desc->max_cores;
                free_cores = desc->spare_cores;
        }

        /* get core count and create accordingly */
        int core_count = lib_cpu_count();

        int thread_count = max_cores < core_count ? max_cores : core_count;
        thread_count -= free_cores;

        if (thread_count < 0) {
                thread_count = 2;
        }

        thread_count = 3;
        new_ctx->thread_count = thread_count;

        LIB_LOG_INFO(
                "Job Dispatcher: Core Count %d, Thread Count %d",
                core_count,
                thread_count);

        /* create signals */
        new_ctx->signal_start = lib_signal_create();
        new_ctx->signal_new_work = lib_signal_create();

        /* create threads */
        {
                lib_array_create_with_capacity(new_ctx->tls, thread_count);
                lib_array_create_with_capacity(new_ctx->threads, thread_count);
                lib_array_create_with_capacity(new_ctx->thread_ids, thread_count);

                int i;

                for (i = 0; i < thread_count; ++i) {
                
                        /* new tls and tls setup */
                        {
                                struct thread_local_storage new_tls;
                                LIB_MEM_ZERO(new_tls);

                                lib_spin_lock_init(&new_tls.job_lock);
                                lib_spin_lock_init(&new_tls.fiber_lock);

                                lib_array_create_with_capacity(new_tls.batches, 64);
                                lib_array_create_with_capacity(new_tls.batch_ids, 64);
                                lib_array_create_with_capacity(new_tls.pending_jobs, 256);
                                lib_array_create_with_capacity(new_tls.log, 2048);

                                unsigned fiber_count = 32;

                                lib_array_create_with_capacity(new_tls.blocked_fibers, fiber_count);
                                lib_array_create_with_capacity(new_tls.blocked_fiber_batch_ids, fiber_count);
                                lib_array_create_with_capacity(new_tls.free_fiber_pool, fiber_count);

                                unsigned j;

                                for (j = 0; j < fiber_count; ++j) {
                                
                                        struct roa_fiber *fi;
                                        roa_fiber_create(&fi, fiber_process, (void*)new_ctx);

                                        lib_array_push(new_tls.free_fiber_pool, fi);
                                }

                                lib_array_push(new_ctx->tls, new_tls);
                        }

                        /* thread proc will fill this in */
                        {
                                roa_thread_id id;
                                LIB_MEM_ZERO(id);
                                lib_array_push(new_ctx->thread_ids, id);
                        }

                        /* create new thread */
                        {
                              roa_thread th;
                              LIB_MEM_ZERO(th);

                                /* thread zero is main thread */
                                if (i != 0) {
                                        struct thread_arg *arg = calloc(sizeof(*arg), 1);
                                        arg->ctx = new_ctx;
                                        arg->tls_id = i;

                                        th = lib_thread_create(thread_process, arg, 1024, i);

                                        if (LIB_IS_ENABLED(ROA_JOB_CPU_AFFINITY)) {
                                                lib_thread_set_affinity(th, i);
                                        }
                                }
                                else {
                                        /* get now so that we can add jobs */
                                        new_ctx->thread_ids[0] = lib_thread_get_current_id();
                                }

                                lib_array_push(new_ctx->threads, th);
                        }
                }
        }

        *ctx = new_ctx;
}


void
tyrant_ctx_run(
        tyrant_ctx_t ctx)
{
        /* param check */
        LIB_ASSERT(ctx);

        /* signal start */
        ctx->threads[0] = lib_thread_create_self();

        if (LIB_IS_ENABLED(ROA_JOB_CPU_AFFINITY)) {
                lib_thread_set_affinity(ctx->threads[0], 0);
        }

        struct thread_arg *arg = calloc(sizeof(*arg), 1);
        arg->ctx = ctx;
        arg->tls_id = 0;

        /* become a dispatcher also */
        thread_process(arg);

        /* quitting ... */
}


void
tyrant_ctx_destroy(
        tyrant_ctx_t *ctx)
{
        /* param check */
        LIB_ASSERT(ctx);
        LIB_ASSERT(*ctx);

        struct tyrant_ctx *kill_ctx = *ctx;

        /* early bail */
        if (!kill_ctx) {
                return;
        }

        /* free threads and data */
        unsigned i, j;
        for (i = 1; i < kill_ctx->thread_count; ++i) {
                if(LIB_IS_ENABLED(ROA_JOB_DEBUG_OUTPUT)) {
                        printf("th destroy %d\n", i);
                }

                lib_thread_destroy(kill_ctx->threads[i]);
        }
        
        /* dump logs */
        #ifndef _WIN32
        FILE *f;
        f = fopen("log.trace", "w");
        fprintf(f, "{\n");
        fprintf(f, "\t\"traceEvents\": [\n");

        int first = 1;

        for (i = 0; i < kill_ctx->thread_count; ++i) {
                for(j = 0; j < lib_array_size(kill_ctx->tls[i].log); ++j) {
                        Dl_info info;
                        const char *name = "unknown";
                        int suc = dladdr(kill_ctx->tls[i].log[j].function, &info);

                        if(suc) {
                                name = info.dli_sname;
                        }

                        fprintf(f, "%s{ \"pid\":1, \"tid\":%d, \"ts\":%llu, \"dur\":%llu, \"ph\":\"X\", \"name\":\"%s\"}",
                                first ? "" : ",\n",
                                i,
                                kill_ctx->tls[i].log[j].ts_start,
                                kill_ctx->tls[i].log[j].ts_end - kill_ctx->tls[i].log[j].ts_start,
                                name
                        );

                        first = 0;
                }
        }
        
        fprintf(f, "\t]\n");
        fprintf(f, "}\n");
        fclose(f);
        #endif

        /* destroy containers */
        lib_array_destroy(kill_ctx->threads);
        lib_array_destroy(kill_ctx->thread_ids);
        lib_array_destroy(kill_ctx->tls);

        free(kill_ctx);
}


void
tyrant_ctx_get_desc(
        tyrant_ctx_t ctx,
        struct tyrant_ctx_query_desc *desc)
{
        (void)ctx;
        (void)desc;
}
