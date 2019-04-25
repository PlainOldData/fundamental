#include <tyrant/tyrant.h>
#include <thread_dispatch/thread_process.h>
#include <thread_dispatch/thread_local_storage.h>
#include <lib/thread.h>
#include <lib/assert.h>
#include <lib/alloc.h>
#include <lib/array.h>
#include <lib/signal.h>
#include <lib/time.h>
#include <lib/spin_lock.h>
#include <ctx/context.h>
#include <fiber/fiber.h>
#include <jobs/jobs.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/* ------------------------------------------------------------ [ config ] -- */


#ifndef ROA_JOB_DEBUG_TH_PROCESS_OUTPUT
#define ROA_JOB_DEBUG_TH_PROCESS_OUTPUT 0
#endif


#ifndef ROA_JOB_DEBUG_NAME_THREADS
#define ROA_JOB_DEBUG_NAME_THREADS 0
#endif


#ifndef ROA_JOB_STEAL_SIZE
#define ROA_JOB_STEAL_SIZE 1
#endif


/* ---------------------------------------------------- [ thread process ] -- */


/*
  Process the fiber.
  when the fiber is switched to it will execute the function with the arg
  provided, then switch back to the home fiber.
*/
void
fiber_process(void *arg)
{
        /* param check */
        LIB_ASSERT(arg);

        /* get ctx */
        struct tyrant_ctx *ctx = (struct tyrant_ctx*)arg;
        LIB_ASSERT(ctx);

        /* one time tls search */
        /*
        if we move to a model where fibers can wake up
        on different threads this needs to change
        */
        int th_index = job_internal_find_thread_index(ctx);
        struct thread_local_storage *tls = &ctx->tls[th_index];

        LIB_ASSERT(th_index > -1);
        LIB_ASSERT(tls->home_fiber);

        while (1) {
                /* exec job */
                {
                        lib_spin_lock_aquire(&tls->fiber_lock);

                        struct job_internal job_data = tls->executing_fiber.desc;
                        struct tyrant_job_desc desc = job_data.desc;

                        tyrant_job_fn job_func = (tyrant_job_fn)desc.func;
                        void *job_arg = desc.arg;

                        lib_spin_lock_release(&tls->fiber_lock);

                        if (LIB_IS_ENABLED(ROA_JOB_DEBUG_TH_PROCESS_OUTPUT)) {
                                char buffer[128];
                                LIB_MEM_ZERO(buffer);

                                sprintf(buffer, "th: %d exec - func: %p arg %p counter %p", th_index, job_func, job_arg, tls->executing_fiber.desc.counter);
                                printf("%s\n", buffer);
                        }

                        job_func(ctx, job_arg);
                }

                /* decrement job counter */
                /* clear executing as its done */
                {
                        /* decrement job count */
                        lib_atomic_int_dec(tls->executing_fiber.desc.counter);

                        if(LIB_IS_ENABLED(ROA_JOB_DEBUG_TH_PROCESS_OUTPUT)) {
                                char buffer[128];
                                LIB_MEM_ZERO(buffer);

                                struct job_internal job_data = tls->executing_fiber.desc;
                                struct tyrant_job_desc desc = job_data.desc;

                                tyrant_job_fn job_func = (tyrant_job_fn)desc.func;
                                void *job_arg = desc.arg;

                                sprintf(buffer, "th: %d done - func: %p arg %p counter %p", th_index, job_func, job_arg, tls->executing_fiber.desc.counter);
                                printf("%s\n", buffer);
                        }
                }

                /* swtich back */
                {
                        lib_spin_lock_aquire(&tls->fiber_lock);

                        struct roa_fiber *worker = tls->executing_fiber.worker_fiber;
                        LIB_ASSERT(worker);

                        struct roa_fiber *home = tls->home_fiber;
                        LIB_ASSERT(home);

                        lib_spin_lock_release(&tls->fiber_lock);

                        roa_fiber_switch(worker, home);
                }
        }
}

/*
  Switches to a pending fiber.
  returns true if a pending fiber was executed.
*/
LIB_BOOL
thread_internal_run_fiber(
        struct thread_local_storage *tls)
{
        LIB_BOOL return_val = LIB_FALSE;

        /* get the current batch ids */
	uint32_t *batch_id = LIB_NULL;
	unsigned batch_count = 0;
	{
		lib_spin_lock_aquire(&tls->job_lock);

		batch_count = lib_array_size(tls->batch_ids);

		if(batch_count) {
			unsigned bytes = sizeof(tls->batch_ids[0]) * batch_count;
			batch_id = malloc(bytes);

			memcpy(batch_id, tls->batch_ids, bytes);
		}

		lib_spin_lock_release(&tls->job_lock);
	}

	/* check batch ids against fibers */
	{
                lib_spin_lock_aquire(&tls->fiber_lock);

		unsigned blocked_count = lib_array_size(tls->blocked_fiber_batch_ids);

		unsigned i,j;
		int unblocked_fiber_index = -1;

	        /* search for some work */
		for (i = 0; i < blocked_count; ++i) {
			uint32_t search_for = tls->blocked_fiber_batch_ids[i];

                        for (j = 0; j < batch_count; ++j) {
		                if (batch_id[j] == search_for) {
                                        /* batch still exists cant execute */
				        break;
	                        }
		        }

                        /* if we got to the end then its unblocked */
			if (j == batch_count) {
		                unblocked_fiber_index = i;
			        break;
                        }
		}

                /* cleanup */
                if (batch_id) {
                        free(batch_id);
                        batch_id = 0;
                }

                /* setup fiber */
	        if (unblocked_fiber_index != -1) {
                        tls->executing_fiber = tls->blocked_fibers[i];

	                lib_array_erase(tls->blocked_fibers, i);
		        lib_array_erase(tls->blocked_fiber_batch_ids, i);
		}

	        /* do we have a fiber? execute it */
		if (tls->executing_fiber.worker_fiber) {
			return_val = 0;

	                /* fiber switch */
		        {
                                struct thread_log log;
                                log.started = unblocked_fiber_index == -1 ? 1 : 0;
                                log.ts_start = lib_time_get_current_ms();
                                
                                struct roa_fiber *worker = tls->executing_fiber.worker_fiber;
				LIB_ASSERT(worker);

	                        /* switch to worker ... */

                                /* release because it might call wait */
                                lib_spin_lock_release(&tls->fiber_lock);

		                roa_fiber_switch(tls->home_fiber, worker);

                                lib_spin_lock_aquire(&tls->fiber_lock);
                                
                                log.ts_end = lib_time_get_current_ms();
                                log.function = tls->executing_fiber.desc.desc.func;
                                
                                lib_array_push(tls->log, log);

			        /* ... back from fiber */
	                }

		        /* free fiber */
			if (tls->executing_fiber.worker_fiber) {
		                lib_array_push(tls->free_fiber_pool, tls->executing_fiber.worker_fiber);
			        tls->executing_fiber.worker_fiber = 0;
	                }
		}

		lib_spin_lock_release(&tls->fiber_lock);
	}

        return return_val;
}


/*
  Removes any batches that counters are no zero, this unblocks any fibers that
  are pending on a batch.

  returns true if any batches where removed.
*/
LIB_BOOL
thread_internal_remove_cleared_batches(
  struct thread_local_storage *tls)
{
        LIB_BOOL return_val = LIB_FALSE;

  lib_spin_lock_aquire(&tls->job_lock);

  unsigned batch_count = lib_array_size(tls->batch_ids);
  unsigned i;
  unsigned erase_index = 0;

  for (i = 0; i < batch_count; ++i)
  {
    struct job_batch *batch = &tls->batches[erase_index];
    int count = lib_atomic_int_load(batch->counter);

    if (count <= 0)
    {
      if(LIB_IS_ENABLED(ROA_JOB_DEBUG_TH_PROCESS_OUTPUT))
      {
        printf("th: %d Batch %d erased - jobs done %d - counter %p\n", tls->th_index, tls->batch_ids[erase_index], batch->count, batch->counter);
      }

      free(batch->counter);
      batch->counter = 0;

      LIB_ASSERT(lib_array_size(tls->batch_ids) == lib_array_size(tls->batches));
      LIB_ASSERT(lib_array_size(tls->batch_ids) > erase_index);

      lib_array_erase(tls->batch_ids, erase_index);
      lib_array_erase(tls->batches, erase_index);

      return_val = LIB_TRUE;
    }
    else
    {
      erase_index += 1;
    }
  }

  lib_spin_lock_release(&tls->job_lock);

  return return_val;
}


/*
  Checks the pending list of jobs, if one is found it will place it on the blocked
  fiber list with no batch, this will get picked up by the process.

  returns true if is pushed a job onto the fiber queue.
*/
LIB_BOOL
thread_internal_check_pending(
        struct thread_local_storage *tls)
{
        LIB_BOOL return_val = LIB_FALSE;

        lib_spin_lock_aquire(&tls->fiber_lock);

        unsigned fiber_size = lib_array_size(tls->free_fiber_pool);

        if (fiber_size) {
                lib_spin_lock_aquire(&tls->job_lock);

                unsigned job_count = lib_array_size(tls->pending_jobs);

                /* do we have a new job? create fiber and push on pending */
                if (job_count) {
                        /* create fiber and push onto the pending list */
                        struct roa_fiber *new_fiber = lib_array_back(tls->free_fiber_pool);
                        lib_array_pop(tls->free_fiber_pool);

                        struct job_internal desc = tls->pending_jobs[0];
                        lib_array_erase(tls->pending_jobs, 0);

                        struct executing_fiber exec;
                        exec.worker_fiber = new_fiber;
                        exec.desc = desc;

                        if(LIB_IS_ENABLED(ROA_JOB_DEBUG_TH_PROCESS_OUTPUT)) {
                                printf("th: %d queued task: fn %p, arg %p, counter %p\n", tls->th_index, desc.desc.func, desc.desc.arg, desc.counter);
                        }

                        lib_array_push(tls->blocked_fiber_batch_ids, 0);
                        lib_array_push(tls->blocked_fibers, exec);

                        return_val = LIB_TRUE;
                }

                lib_spin_lock_release(&tls->job_lock);
        }

        lib_spin_lock_release(&tls->fiber_lock);

        return return_val;
}


/*
  This will look at the other TLS for work todo. It will steal one job from the queue.
  returns true if it stole a job.
*/
LIB_BOOL
thread_internal_steal_jobs(
  struct thread_local_storage *tls,
  struct thread_local_storage *tls_arr,
  unsigned th_count,
  unsigned th_index)
{
        LIB_BOOL return_val = LIB_FALSE;

	struct job_internal steal_job[ROA_JOB_STEAL_SIZE];
  unsigned steal_job_counter = 0;

	/* search for work to steal */
	{
    /*
		if(ROA_IS_ENABLED(ROA_JOB_DEBUG_TH_PROCESS_OUTPUT))
		{
			printf("Thread %d looking to steal \n", th_index);
		}
    */

		unsigned i;

		for(i = 0; i < th_count; ++i)
		{
			int steal_index = ((th_index + 1 + i) % th_count);
			struct thread_local_storage *steal_tls = &tls_arr[steal_index];

			/* shouldn't happen but just incase */
			if(steal_tls == tls)
			{
				continue;
			}

			/* try and get lock - if not move on */
			if(lib_spin_lock_try_aquire(&steal_tls->job_lock))
			{
				unsigned job_count = lib_array_size(steal_tls->pending_jobs);

        unsigned j, k;

        for(j = 0, k = 0; j < job_count; ++j)
				{
					struct job_internal steal = steal_tls->pending_jobs[k];

					if(steal.desc.thread_locked == LIB_FALSE)
					{
						steal_job[steal_job_counter] = steal;
						steal_job_counter += 1;

						lib_array_erase(steal_tls->pending_jobs, k);

						if(LIB_IS_ENABLED(ROA_JOB_DEBUG_TH_PROCESS_OUTPUT))
						{
							printf("th: %d stole %p from thread %d counter %p \n", th_index, steal.desc.func, steal_index, steal.counter);
						}

            if(steal_job_counter >= ROA_JOB_STEAL_SIZE)
            {
						  break;
            }

            job_count -= 1;
					}
          else
          {
            k += 1;
          }
				}

				lib_spin_lock_release(&steal_tls->job_lock);

        if (steal_job_counter)
        {
          break;
        }
			}
		}
	}

	if(steal_job_counter)
	{
    /* thread lock so it doesn't bounce around getting stolen */
    return_val = LIB_TRUE;

    /* lock this tls jobs and apply work */
    {
		  lib_spin_lock_aquire(&tls->job_lock);

      unsigned i;

      for(i = 0; i < steal_job_counter; ++i)
      {
        steal_job[i].desc.thread_locked = LIB_TRUE;
		    lib_array_push(tls->pending_jobs, steal_job[i]);
      }

		  lib_spin_lock_release(&tls->job_lock);
    }
	}

  return return_val;
}


/*
  If this is the main thread it will check the other threads to see if they have work.
  if not it will signal quit. If this is a worker thread then it will wait until new work
  signal.

  returns true if dispatcher should keep going.
*/
LIB_BOOL
thread_internal_wait_or_quit(
        struct tyrant_ctx *ctx,
        struct thread_local_storage *tls,
        struct thread_local_storage *tls_arr,
        unsigned th_count,
        unsigned th_index)
{
        LIB_BOOL stay_alive = LIB_TRUE;

        /* if main thread check other threads */
        if (th_index == 0) {
                unsigned i;
                LIB_BOOL work = LIB_FALSE;

                /* lock all thread jobs */
                for(i = 0; i < th_count; ++i) {
                        lib_spin_lock_aquire(&tls_arr[i].job_lock);
                }

                /* check batches */
                for(i = 0; i < th_count; ++i) {
                        unsigned count = lib_array_size(tls_arr[i].batches);

                        if(count) {
                                work = LIB_TRUE;
                                break;
                        }
                }

                /* unlock all */
                for(i = 0; i < th_count; ++i) {
                        lib_spin_lock_release(&tls_arr[i].job_lock);
                }

                /* signal quit if required */
                if(work == LIB_FALSE) {
                        for(i = 0; i < th_count; ++i) {
                                if(LIB_IS_ENABLED(ROA_JOB_DEBUG_TH_PROCESS_OUTPUT)) {
                                        printf("th: %d quit\n", i);
                                }

                                tls_arr[i].thread_status = TLS_QUIT;
                        }

                        stay_alive = LIB_FALSE;
                }
        }
	else if(tls->thread_status == TLS_QUIT) {
		stay_alive = LIB_FALSE;
	}
        else {
                lib_signal_wait(ctx->signal_new_work, 1000);
        }

        return stay_alive;
}


/*
  Runs the thread.
  It will in order check each stage, should any stage complete, it will start again from
  the begining.

  if it gets to the end the thread will exit.
*/
void*
thread_process(void *arg)
{
  /* param check */
  LIB_ASSERT(arg);
  
  struct thread_local_storage *tls = 0;
  struct tyrant_ctx *ctx;
  int th_index = -1;
  int th_count = 0;

  /* get/set data setup */
  {
    struct thread_arg *th_arg = (struct thread_arg*)arg;
    ctx = th_arg->ctx;
    th_index = th_arg->tls_id;
    tls = &ctx->tls[th_arg->tls_id];
    th_count = lib_array_size(ctx->tls);
    ctx->thread_ids[th_index] = lib_thread_get_current_id();
    
    free(arg);
    arg = 0;
  }

  if(LIB_IS_ENABLED(ROA_JOB_DEBUG_NAME_THREADS))
  {
    lib_thread_set_current_name("Tyrant Job Thread");
  }

  roa_fiber_create(&tls->home_fiber, LIB_NULL, 0);

  /*
    first check pending fibers, if found switch to.
    then check pending jobs, if found create pending fiber.
    then try steal work from other threads, and create a pending job.
  */

  /* wait until signal */
  if(th_index > 0)
  {
    lib_signal_wait(ctx->signal_start, 1);
  }
  else
  {
    lib_signal_raise(ctx->signal_start);
  }

  /* if this loop gets to the bottom it exits */
  while (tls->thread_status != TLS_QUIT)
  {
    /* remove completed batches */
    if (thread_internal_remove_cleared_batches(tls))
    {
      continue;
    }

    /* check pending fibers */
    if (thread_internal_run_fiber(tls))
    {
      continue;
    }

    /* check pending jobs */
    if (thread_internal_check_pending(tls))
    {
      continue;
    }

    /* steal jobs - since nothing else todo :) */
    if (thread_internal_steal_jobs(
          tls,
          ctx->tls,
          th_count,
          th_index))
    {
      continue;
    }

    if (thread_internal_wait_or_quit(
          ctx,
          tls,
          ctx->tls,
          th_count,
          th_index))
    {
      continue;
    }

		if(LIB_IS_ENABLED(ROA_JOB_DEBUG_TH_PROCESS_OUTPUT))
		{
			printf("Exiting th %d\n", th_index);
		}

  } /* while processing */

  return LIB_NULL;
}


/*
  Helper to find where in the tls array the callee is.
*/
int
job_internal_find_thread_index(struct tyrant_ctx *ctx)
{
  roa_thread_id this_th = lib_thread_get_current_id();

  unsigned i;
  int th_index = -1;

  while(th_index < 0) {
    for (i = 0; i < ctx->thread_count; ++i)
    {
      if (ctx->thread_ids[i] == this_th)
      {
        th_index = (int)i;
        break;
      }
    }
  }

  return th_index;
}
