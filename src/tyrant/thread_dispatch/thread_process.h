#ifndef THREAD_PROCESS_INCLUDED_7EF312E5_B2FF_4DAC_ABEA_BEE7CA47ED13
#define THREAD_PROCESS_INCLUDED_7EF312E5_B2FF_4DAC_ABEA_BEE7CA47ED13


/* 
  Loop that runs on a fiber.
  Pass the dispatcher ctx to it.
*/
void
fiber_process(void *arg);


struct tyrant_ctx;


/* pass an instance of this structure to the thread_process */
/* it will be freed by the process */
struct thread_arg
{
        int tls_id;
        struct tyrant_ctx *ctx;
};

/*
  Loop that runs on a thread.
  pass thread_arg to it.
*/
void*
thread_process(void *arg);


/*
  Helper to locate which tls you need.
*/
int
job_internal_find_thread_index(struct tyrant_ctx *ctx);


#endif /* inc guard */
