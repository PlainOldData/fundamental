#ifndef ATOMIC_INCLUDED_E1ECD72F_192A_4EB2_B939_1E4AD5E8B964
#define ATOMIC_INCLUDED_E1ECD72F_192A_4EB2_B939_1E4AD5E8B964


#ifdef __cplusplus
extern "C" {
#endif


/* ------------------------------------------------------ [ Atomic Types ] -- */


typedef union lib_atomic_int 
{
	void* align;
	int val;
} lib_atomic_int;


typedef union lib_atomic_ptr 
{
	void* ptr;
} lib_atomic_ptr;


/* ---------------------------------------------- [ Atomic Int Interface ] -- */


int
lib_atomic_int_load(
	lib_atomic_int *atomic);


void
lib_atomic_int_store(
	lib_atomic_int *atomic,
	int val);


int
lib_atomic_int_inc(
	lib_atomic_int *atomic);


int
lib_atomic_int_dec(
	lib_atomic_int *atomic);


int
lib_atomic_int_add(
	lib_atomic_int *atomic,
	int add);


int
lib_atomic_int_sub(
	lib_atomic_int *atomic,
	int sub);


int
lib_atomic_int_swap(
	lib_atomic_int *atomic,
	int swap);


int
lib_atomic_int_compare_and_swap(
	lib_atomic_int *atomic,
	int old_value,
	int new_value);


/* ---------------------------------------------- [ Atomic Ptr Interface ] -- */


void*
lib_atomic_ptr_load(
	lib_atomic_ptr *tomic);


void
lib_atomic_ptr_store(
	lib_atomic_ptr *atomic,
	void *value);


void*
lib_atomic_ptr_swap(
	lib_atomic_ptr *atomic,
	void *value);


void*
lib_atomic_ptr_compare_and_swap(
	lib_atomic_ptr *atomic,
	void *old_value,
	void *new_value);


#ifdef __cplusplus
} /* extern */
#endif


#endif /* inc guard */