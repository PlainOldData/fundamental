#include <tyrant/tyrant.h>
#include <lib/fundamental.h>
#include <lib/assert.h>
#include <lib/alloc.h>
#include <lib/atomic.h>
#include <lib/time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/* -------------------------------------------------------------- [ Vars ] -- */


#define TEST_WITH_OUTPUT 1


#define BATCH_COUNT (1 << 6)
#define TICK_COUNT (1 << 6)


int ticks = TICK_COUNT;
lib_atomic_int *test_data;


/* --------------------------------------------------------------- [ Fwd ] -- */


void submit_tick(tyrant_ctx_t ctx);


/* --------------------------------------------------------- [ Test Jobs ] -- */


void
calculate(tyrant_ctx_t ctx, void *arg)
{
        (void)ctx;
        (void)arg;

        if (LIB_IS_ENABLED(TEST_WITH_OUTPUT)) {
        printf("calc job\n");
        }

        lib_atomic_int *int_arg = (lib_atomic_int*)arg;
        lib_atomic_int_inc(int_arg);

        int expected = (TICK_COUNT - ticks);
        int data = lib_atomic_int_load(int_arg);

        LIB_ASSERT(data == expected);
}


void
tick(tyrant_ctx_t ctx, void *arg)
{
        (void)arg;

        if (LIB_IS_ENABLED(TEST_WITH_OUTPUT)) {
                printf("tick %d\n", TICK_COUNT - ticks);
        }

        /* submit batch of jobs */
        {
                int i;

                struct tyrant_job_desc *batch = 0;
                unsigned batch_bytes = sizeof(*batch) * BATCH_COUNT;
                batch = malloc(batch_bytes);

                for (i = 0; i < BATCH_COUNT; ++i) {
                        batch[i].func = calculate;
                        batch[i].arg = (void*)&test_data[i];
                        batch[i].thread_locked = 0;
                }

                uint64_t batch_id = tyrant_job_submit(
                        ctx,
                        batch,
                        BATCH_COUNT);

                tyrant_job_wait(ctx, batch_id);
                free(batch);
        }

        /* check test data is correct */
        {
                int i;

                for (i = 0; i < BATCH_COUNT; ++i) {
                        int expected = (TICK_COUNT - ticks);
                        int data = lib_atomic_int_load(&test_data[i]);
                        LIB_ASSERT(data == expected);
                }
        }

        /* another tick? */
        if (ticks > 0) {
                submit_tick(ctx);
        }
}


/* ------------------------------------------------------- [ Helper Func ] -- */


void
submit_tick(tyrant_ctx_t ctx)
{
        struct tyrant_job_desc desc[1];
        desc[0].func = tick;
        desc[0].arg = (void*)&ticks;
        desc[0].thread_locked = LIB_FALSE;

        tyrant_job_submit(ctx, LIB_ARR_DATA(desc), LIB_ARR_COUNT(desc));

        ticks -= 1;
}


/* -------------------------------------------------------------- [ Main ] -- */


int
main()
{
        /* double check test data */
        unsigned max_value = (unsigned)-1;
        unsigned tick = TICK_COUNT;
        LIB_ASSERT(max_value > tick);

        tyrant_ctx_t ctx = 0;
        tyrant_ctx_create(&ctx, 0);

        test_data = calloc(sizeof(test_data[0]) * BATCH_COUNT, 1);

        /* start time */
        unsigned long start = lib_time_get_current_ms();

        submit_tick(ctx);
        tyrant_ctx_run(ctx);

        /* time taken */
        unsigned long end = lib_time_get_current_ms();
        unsigned long time = end - start;

        /* back from dispatcher */
        tyrant_ctx_destroy(&ctx);


        if (LIB_IS_ENABLED(TEST_WITH_OUTPUT)) {
                printf("Time taken %lu\n", time);
        }

	if (LIB_IS_ENABLED(TEST_WITH_OUTPUT)) {
                int i;

                printf("Test Data \n");

                for (i = 0; i < BATCH_COUNT; ++i) {
                        printf("%d, ", test_data[i].val);
                }

                printf("\n");
        }

        /* check test data is correct */
        {
                int i;

                for (i = 0; i < BATCH_COUNT; ++i) {
                        int expected = TICK_COUNT;
                        LIB_ASSERT(lib_atomic_int_load(&test_data[i]) == expected);

                        if (lib_atomic_int_load(&test_data[i]) != expected) {
                                exit(EXIT_FAILURE);
                        }
                }
        }

  return 0;
}
