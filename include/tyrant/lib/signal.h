#ifndef SIGNAL_INCLUDED_3E3DE28D_72DD_4135_8961_6DA5A3CFDA70
#define SIGNAL_INCLUDED_3E3DE28D_72DD_4135_8961_6DA5A3CFDA70


#ifdef __cplusplus
extern "C" {
#endif


/* ------------------------------------------------------- [ Signal Type ] -- */


typedef void* lib_signal;


/* --------------------------------------------------- [ Signal Lifetime ] -- */


lib_signal
lib_signal_create();


void
lib_signal_destroy(lib_signal s);


/* ---------------------------------------------------- [ Signal Actions ] -- */


void
lib_signal_raise(lib_signal s);


int
lib_signal_wait(lib_signal s, int timeout_ms);


#ifdef __cplusplus
} /* extern */
#endif


#endif /* inc guard */