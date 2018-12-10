#ifndef ARRAY_INCLUDED
#define ARRAY_INCLUDED


#ifndef __cplusplus
#include <stddef.h>
#else
#include <cstddef>
#endif


/*
  C is a little awkward for containers, but this is a good balance for arrays.
  It uses macros unforunatly but it means we have a typesafe buffer.
  int *array = NULL;
  pd_array_create(array);
  pd_array_push(array, 123);
*/


typedef void*(*pd_array_alloc_fn)(size_t);
typedef void(*pd_array_free_fn)(void*);


struct pd_array_details_desc {
        void *buffer;                       /* optional */
        int buffer_size;
        int protect_buffer;
        pd_array_alloc_fn alloc_fn;         /* if null this array will not resize/destroy pass malloc if unsure */ 
        pd_array_free_fn free_fn;           /* if null this array will not resize/destroy pass free if unsure */
};


#define pd_array_create_with_details(arr, details) do { _pdi_arr_create_with_buffer((void**)&arr, sizeof(arr[0]), buf, buf_size); } while(0);
#define pd_array_create(arr)                       do { _pdi_arr_create((void**)&arr, sizeof(arr[0]), 1); } while(0)
#define pd_array_create_with_capacity(arr, cap)    do { _pdi_arr_create((void**)&arr, sizeof(arr[0]), cap); } while(0)
#define pd_array_destroy(arr)                      do { _pdi_arr_destroy((void**)&arr); } while(0)
#define pd_array_size(arr)                         _pdi_arr_size((void**)&arr, sizeof(arr[0]))
#define pd_array_capacity(arr)                     _pdi_arr_capacity((void**)&arr, sizeof(arr[0]))
#define pd_array_resize(arr, cap)                  do { _pdi_arr_resize((void**)&arr, cap, sizeof(arr[0])); } while(0)
#define pd_array_push(arr, item)                   do { _pdi_arr_push((void**)&arr, sizeof(arr[0])); arr[pd_array_size(arr) - 1] = item;} while(0)
#define pd_array_erase(arr, index)                 do { _pdi_arr_erase((void**)&arr, index, sizeof(arr[0])); } while(0)
#define pd_array_pop(arr)                          do { _pdi_arr_pop((void**)&arr, sizeof(arr[0])); } while(0)
#define pd_array_insert(arr, i, item)              do { _pdi_arr_insert((void**)&arr, i, sizeof(arr[0])); arr[i] = item;} while(0)
#define pd_array_back(arr)                         arr[pd_array_size(arr) - 1]
#define pd_array_clear(arr)                        do { _pdi_arr_clear((void**)&arr); } while(0)
#define pd_array_data(arr)                         &arr[0]


/* ------------------------------------------------ Stretchy Array Private -- */
/*
  These are internal you shouldn't be calling them directly.
*/


void        _pdi_arr_create_with_details(void **ptr, unsigned stride, struct pd_array_details_desc *desc);
void        _pdi_arr_create(void **ptr, unsigned stride, unsigned capacity);
void        _pdi_arr_destroy(void **ptr);
unsigned    _pdi_arr_size(void **ptr, unsigned stride);
unsigned    _pdi_arr_capacity(void **ptr, unsigned stride);
void        _pdi_arr_resize(void **ptr, unsigned size, unsigned stride);
unsigned    _pdi_arr_push(void **ptr, unsigned stride);
void        _pdi_arr_erase(void **ptr, unsigned index, unsigned stride);
void        _pdi_arr_pop(void **ptr, unsigned stride);
unsigned    _pdi_arr_insert(void **ptr, unsigned index, unsigned stride);
void        _pdi_arr_clear(void**ptr);


#endif


/* ================================== IMPL ================================== */


#ifdef ARRAY_IMPL
#ifndef ARRAY_IMPL_INCLUDED
#define ARRAY_IMPL_INCLUDED


#ifdef __cplusplus
#include <cstdlib>
#include <cstring>
#else
#include <stdlib.h>
#include <string.h>
#endif


/* ---------------------------------------------------------------- Config -- */


/* turn this on if you suspect something in array */
#ifndef NDEBUG
#define KC_ARR_PEDANTIC_CHECKS 0
#else
#define KC_ARR_PEDANTIC_CHECKS 1
#endif


#define KC_ASSERT(expr)


struct kc_array_header {
        unsigned char * capacity;
        unsigned char * end;
        pd_array_alloc_fn alloc_fn;
        pd_array_free_fn free_fn;
};


void*
_pdi_arr_grow(void **ptr, unsigned stride, unsigned capacity)
{
        if (KC_ARR_PEDANTIC_CHECKS) {
                KC_ASSERT(ptr);
                KC_ASSERT(*ptr);
                KC_ASSERT(stride);
                KC_ASSERT(capacity);
        }

        /* increase buffer */
        struct kc_array_header *header = ((struct kc_array_header*)*ptr);
        header--;

        unsigned count = _pdi_arr_size(ptr, stride);
        unsigned bytes = (stride * capacity) + sizeof(struct kc_array_header);
        
        header = (struct kc_array_header*)realloc(header, bytes);

        unsigned char *begin = (unsigned char*)&header[1];

        header->end = begin + (stride * count);
        header->capacity = begin + (stride * capacity);

        *ptr = &header[1];

        return header;
}


void
_pdi_arr_create_with_details(
        void **ptr,
        unsigned stride,
        struct pd_array_details_desc * desc)
{
        if (KC_ARR_PEDANTIC_CHECKS) {
                KC_ASSERT(ptr);
                KC_ASSERT(*ptr == 0);
                KC_ASSERT(desc);
        }

        struct kc_array_header *header = 0;
        
        if(desc->buffer) {
                header = desc->buffer; 
        } else {
                header = desc->alloc_fn(stride * 32 + sizeof(*header));
        }
        unsigned char *begin = (unsigned char*)&header[1];

        header[0].end = begin;
        header[0].capacity = (unsigned char*)header + desc->buffer_size;

        *ptr = (void*)begin;
}


void
_pdi_arr_create(
        void **ptr,
        unsigned stride,
        unsigned capacity)
{
        if (capacity == 0) {
                capacity = 1;
        }

        unsigned array_bytes = stride * capacity;
        unsigned bytes = sizeof(struct kc_array_header) + array_bytes;

        void *buffer = malloc(bytes);

        struct pd_array_details_desc details = {0};
        details.buffer = buffer;
        details.buffer_size = bytes;
        details.alloc_fn = malloc;
        details.free_fn = free;

        _pdi_arr_create_with_details(ptr, stride, &details);

        /*
        unsigned array_bytes = stride * capacity;
        unsigned bytes = sizeof(struct kc_array_header) + array_bytes;
        struct kc_array_header *header = malloc(bytes);
        unsigned char *begin = (unsigned char*)&header[1];
        header[0].end = begin;
        header[0].capacity = begin + array_bytes;
        *ptr = (void*)begin;
        */
}


void
_pdi_arr_destroy(void **ptr)
{
        if (KC_ARR_PEDANTIC_CHECKS) {
                KC_ASSERT(ptr);
                KC_ASSERT(*ptr);
        }

        struct kc_array_header *header = ((struct kc_array_header*)*ptr);
        header--;

        free(header);
        *ptr = 0;
}


unsigned
_pdi_arr_size(void **ptr, unsigned stride)
{
        if (KC_ARR_PEDANTIC_CHECKS) {
KC_ASSERT(ptr);
                KC_ASSERT(*ptr);
                KC_ASSERT(stride);
        }

        struct kc_array_header *header = ((struct kc_array_header*)*ptr);
        header--;

        unsigned char * begin = ((unsigned char*)*ptr);
        unsigned count = (header->end - begin) / stride;

        return count;
}


unsigned
_pdi_arr_capacity(void **ptr, unsigned stride)
{
        if (KC_ARR_PEDANTIC_CHECKS) {
                KC_ASSERT(ptr);
                KC_ASSERT(*ptr);
                KC_ASSERT(stride);
        }

        struct kc_array_header *header = ((struct kc_array_header*)*ptr);
        header--;

        return (header->capacity - ((unsigned char*)*ptr)) / stride;
}


void
_pdi_arr_resize(void **ptr, unsigned size, unsigned stride)
{
        if (KC_ARR_PEDANTIC_CHECKS) {
                KC_ASSERT(ptr);
                KC_ASSERT(*ptr);
                KC_ASSERT(size);
                KC_ASSERT(stride);
        }

        struct kc_array_header *header = ((struct kc_array_header*)*ptr);
        header--;

        header = _pdi_arr_grow(ptr, stride, size);
        header->end = header->capacity;
}


void
_pdi_arr_erase(void **ptr, unsigned index, unsigned stride)
{
        if (KC_ARR_PEDANTIC_CHECKS) {
                KC_ASSERT(ptr);
                KC_ASSERT(*ptr);
                KC_ASSERT(stride);
        }

        struct kc_array_header *header = ((struct kc_array_header*)*ptr);
        header--;

        unsigned char *dst = &((unsigned char*)(*ptr))[stride * index];
        unsigned char *src = &((unsigned char*)(*ptr))[stride * (index + 1)];

        unsigned count = _pdi_arr_size(ptr, stride);

        if (count > 0) {
                unsigned len = (stride * (count - 1)) - (stride * index);
                memmove(dst, src, len);

                header->end -= stride;
        }
}


void
_pdi_arr_pop(void **ptr, unsigned stride)
{
        if (KC_ARR_PEDANTIC_CHECKS) {
                KC_ASSERT(ptr);
                KC_ASSERT(*ptr);
        }

        unsigned char *begin = (unsigned char*)*ptr;

        struct kc_array_header *header = ((struct kc_array_header*)*ptr);
        header--;

        if (begin < header->end) {
                header->end -= stride;
        }
}


void
_pdi_arr_clear(void**ptr)
{
        if (KC_ARR_PEDANTIC_CHECKS) {
                KC_ASSERT(ptr);
                KC_ASSERT(*ptr);
        }

        struct kc_array_header *header = ((struct kc_array_header*)*ptr);
        header--;

        unsigned char *begin = (unsigned char*)&header[1];

        header->end = begin;
}



unsigned
_pdi_arr_push(void **ptr, unsigned stride)
{
        if (KC_ARR_PEDANTIC_CHECKS) {
                KC_ASSERT(ptr);
                KC_ASSERT(*ptr);
                KC_ASSERT(stride);
        }

        struct kc_array_header *header = ((struct kc_array_header*)*ptr);
        header--;

        if (header->end >= header->capacity) {
                unsigned count = _pdi_arr_size(ptr, stride);
                header = (struct kc_array_header *)_pdi_arr_grow(
                        ptr,
                        stride,
                        count * 2
                );
        }

        unsigned char * begin = (unsigned char*)(&header[1]);

        /* increment end */
        unsigned index = (header->end - begin) / stride;
        header->end += stride;

        return index;
}


unsigned
_pdi_arr_insert(void **ptr, unsigned index, unsigned stride)
{
        if (KC_ARR_PEDANTIC_CHECKS) {
                KC_ASSERT(ptr);
                KC_ASSERT(*ptr);
                KC_ASSERT(stride);
        }

        /* increase the size to make space */
        _pdi_arr_push(ptr, stride);

        struct kc_array_header *header = ((struct kc_array_header*)*ptr);
        header--;

        unsigned count = _pdi_arr_size(ptr, stride);

        unsigned char * begin = (unsigned char*)(*ptr);

        unsigned char *dst = &begin[stride * (index + 1)];
        unsigned char *src = &begin[stride * index];

        unsigned len = (count - index - 1) * stride;

        memmove(dst, src, len);

        return index;
}


void
_pdi_arr_should_grow(void **ptr, unsigned stride)
{
        if (KC_ARR_PEDANTIC_CHECKS) {
                KC_ASSERT(ptr);
                KC_ASSERT(*ptr);
                KC_ASSERT(stride);
        }

        struct kc_array_header *curr_arr = ((struct kc_array_header*)*ptr);
        curr_arr--;

        unsigned count = _pdi_arr_size(ptr, stride) + 1;

        if (count > _pdi_arr_capacity(ptr, stride)) {
                _pdi_arr_grow(ptr, stride, count * 2);
        }
}

#endif
#endif


