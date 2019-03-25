#ifndef TYRANT_DISPATCHER_INCLUDED_F14B5BF7_A2A2_4A6F_B5DD_E66FB239C8EA
#define TYRANT_DISPATCHER_INCLUDED_F14B5BF7_A2A2_4A6F_B5DD_E66FB239C8EA


#include <lib/fundamental.h>


#ifdef __cplusplus
extern "C" {
#endif


/* ------------------------------------------------------------- [ types ] -- */


typedef struct tyrant_ctx * tyrant_ctx_t;


/* ---------------------------------------------------------- [ lifetime ] -- */


struct tyrant_ctx_setup_desc {
        int type_id;
        void *ext;

        int max_cores;
        int spare_cores;
};


void
tyrant_ctx_create(
        tyrant_ctx_t *ctx,
        struct tyrant_ctx_setup_desc *desc);


void
tyrant_ctx_run(
        tyrant_ctx_t ctx);


void
tyrant_ctx_destroy(
        tyrant_ctx_t *ctx);
  

struct tyrant_ctx_query_desc {
        int type_id;
        void *ext;

        int sizeof_thread_pool;
};


void
tyrant_ctx_get_desc(
        tyrant_ctx_t ctx,
        struct tyrant_ctx_query_desc *desc);


/* -------------------------------------------------------------- [ Jobs ] -- */


#define ROA_JOB_DECL(job_name)                                            \
void job_name(tyrant_ctx_t job_ctx, void *void_arg);                      \

#define ROA_JOB(job_name, arg_type)                                       \
void job_wrap_##job_name(tyrant_ctx_t job_ctx, arg_type arg);             \
                                                                          \
void                                                                      \
job_name(tyrant_ctx_t job_ctx, void *void_arg)                            \
{                                                                         \
  job_wrap_##job_name(job_ctx, (arg_type)void_arg);                       \
}                                                                         \
                                                                          \
void job_wrap_##job_name(tyrant_ctx_t job_ctx, arg_type arg)


typedef void(*tyrant_job_fn)(tyrant_ctx_t ctx, void *arg);


struct tyrant_job_desc {
        tyrant_job_fn  func;
        void*       arg;
        LIB_BOOL    thread_locked;
};


uint64_t
tyrant_job_submit(
        tyrant_ctx_t ctx,
        struct tyrant_job_desc *desc,
        int count);


void
tyrant_job_wait(
        tyrant_ctx_t ctx,
        uint64_t job_marker);


#ifdef __cplusplus
} /* extern */
#endif


#endif /* inc guard */
