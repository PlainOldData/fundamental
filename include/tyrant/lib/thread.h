#ifndef THREAD_INCLUDED_620C4CB9_916F_4A5E_8389_DC5D456E21A5
#define THREAD_INCLUDED_620C4CB9_916F_4A5E_8389_DC5D456E21A5


#ifdef __cplusplus
extern "C" {
#endif


/* ------------------------------------------------------ [ Thread Types ] -- */


typedef void* lib_thread;
typedef void* lib_thread_id;

typedef void*(*lib_thread_func)(void *);


/* --------------------------------------------------- [ Thread Lifetime ] -- */
/*
  Thread lifetime create and destroy.
*/


lib_thread
lib_thread_create(
	lib_thread_func func,
	void *arg,
	int stack_size,
	int core_affinity);


lib_thread
lib_thread_create_self();


void
lib_thread_destroy(lib_thread th);


/* ---------------------------------------------------- [ Thread Actions ] -- */
/*
  Various actions
*/


void
lib_thread_set_affinity(lib_thread th, int affinity);


void
lib_thread_join(lib_thread th);


/* -------------------------------------------------- [ Thread Utilities ] -- */


void
lib_thread_exit_current();


lib_thread_id
lib_thread_get_current_id();


void
lib_thread_set_current_name(const char *name);


void
lib_thread_get_current_name(char *buffer, unsigned buf_size);


unsigned
lib_thread_core_count();


#ifdef __cplusplus
} /* extern */
#endif


#endif /* inc guard */
