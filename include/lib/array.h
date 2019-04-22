#ifndef ARRAY_INCLUDED_8DEFA288_CEAB_4CBA_A7BE_FAF80132D7C1
#define ARRAY_INCLUDED_8DEFA288_CEAB_4CBA_A7BE_FAF80132D7C1


#ifdef __cplusplus
extern "C" {
#endif


/* ------------------------------------------------------------- Utilities -- */


#define ARR_DATA(arr) &arr[0]
#define ARR_COUNT(arr) (sizeof(arr)/sizeof(arr[0]))


/* --------------------------------------------------- [ Array Interface ] -- */
/*
  C is a little awkward for containers, but this is a good balance for arrays.
  It uses macros unforunatly but it means we have a typesafe buffer.

  int *array = NULL;
  codex_array_create(array);
  codex_array_push(array, 123);
*/


#define lib_array_create(arr)											do { lib_internal_array_create((void**)&arr, sizeof(arr[0]), 1); } while(0)
#define lib_array_create_with_capacity(arr, cap)	do { lib_internal_array_create((void**)&arr, sizeof(arr[0]), cap); } while(0)
#define lib_array_destroy(arr)										do { lib_internal_array_destroy((void**)&arr); } while(0)
#define lib_array_size(arr)												lib_internal_array_size((void**)&arr, sizeof(arr[0]))
#define lib_array_capacity(arr)										lib_internal_array_capacity((void**)&arr, sizeof(arr[0]))
#define lib_array_resize(arr, cap)								do { lib_internal_array_resize((void**)&arr, cap, sizeof(arr[0])); } while(0)
#define lib_array_push(arr, item)									do { lib_internal_array_push((void**)&arr, sizeof(arr[0])); arr[lib_array_size(arr) - 1] = item;} while(0)
#define lib_array_erase(arr, index)								do { lib_internal_array_erase((void**)&arr, index, sizeof(arr[0])); } while(0)
#define lib_array_pop(arr)												do { lib_internal_array_pop((void**)&arr, sizeof(arr[0])); } while(0)
#define lib_array_insert(arr, i, item)						do { lib_internal_array_insert((void**)&arr, i, sizeof(arr[0])); arr[i] = item;} while(0)
#define lib_array_back(arr)												arr[lib_array_size(arr) - 1]
#define lib_array_clear(arr)											do { lib_internal_array_clear((void**)&arr); } while(0)
#define lib_array_data(arr)												arr


/* -------------------------------------------------------- [ Array Impl ] -- */
/*
  These are internal you shouldn't be calling them directly.
*/

void        lib_internal_array_create(void **ptr, unsigned stride, unsigned capacity);
void        lib_internal_array_destroy(void **ptr);
unsigned    lib_internal_array_size(void **ptr, unsigned stride);
unsigned    lib_internal_array_capacity(void **ptr, unsigned stride);
void        lib_internal_array_resize(void **ptr, unsigned size, unsigned stride);
unsigned    lib_internal_array_push(void **ptr, unsigned stride);
void        lib_internal_array_erase(void **ptr, unsigned index, unsigned stride);
void        lib_internal_array_pop(void **ptr, unsigned stride);
unsigned    lib_internal_array_insert(void **ptr, unsigned index, unsigned stride);
void        lib_internal_array_clear(void**ptr);


#ifdef __cplusplus
} /* extern */
#endif

#endif /* inc guard */
