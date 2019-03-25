#ifndef ALLOC_INCLUDED_FA713983_DB0B_4D7D_8CC5_00923C16BA74
#define ALLOC_INCLUDED_FA713983_DB0B_4D7D_8CC5_00923C16BA74


#include <lib/fundamental.h>


#ifdef __cplusplus
extern "C" {
#endif


enum lib_allocator_struct {
        LIB_ALLOCATOR_STRUCT_CREATE,
        LIB_ALLOCATOR_STRUCT_UPDATE,

        LIB_ALLOCATOR_STRUCT_TAGGED_DATA,
        LIB_ALLOCATOR_STRUCT_TAGGED_CLEAR,
};


enum lib_allocator_type {
        LIB_ALLOCATOR_MANUAL,
        LIB_ALLOCATOR_TAGGED,
};


struct lib_allocator {
        uint64_t alloc_id;
        uint64_t category_id;
};


struct lib_allocator_create_desc {
        int type_id;
        void *extension;
  
        const char *category_name;
        int type;
};


LIB_STATUS
lib_allocator_create(
        struct lib_allocator_create_desc *desc,
        struct lib_allocator *allocator,
        int count);
  

LIB_STATUS
lib_allocator_destroy(
        struct lib_allocator *allocator);

  
enum lib_allocator_action {
        LIB_ALLOCATOR_PURGE_TAG,
};
  
  
struct lib_allocator_tagged_data {
        int type_id;
        void *ext;

        int allocated_pools;
        int in_use_pools;
        int total_pools;
};


LIB_STATUS
lib_allocator_tagged_get_data(
        struct lib_allocator_tagged_data *data);


struct lib_allocator_tagged_clear {
        int type_id;
        void *ext;

        const char *category_to_clear;
};


LIB_STATUS
lib_allocator_tagged_clear_category(
        struct lib_allocator_tagged_clear *clear);
  
  
void*
lib_alloc(struct lib_allocator *allocator, unsigned bytes);


void*
lib_zalloc(struct lib_allocator *allocator, unsigned bytes);


void
lib_free(struct lib_allocator *allocator, void *addr);


/* --------------------------------------------------------- [ Allocator ] -- */


struct lib_tagged_allocator
{
  uint64_t allocator_tag;

  unsigned block_frame_counter;
  uint8_t *block_begin;
  uint8_t *block_end;
};


void
lib_tagged_allocator_init();


void
lib_tagged_allocator_destroy();


void
lib_tagged_allocator_create(
  struct lib_tagged_allocator *allocator,
  uint64_t tag);


void*
lib_tagged_allocator_alloc(
  struct lib_tagged_allocator *allocator,
  unsigned bytes);


void*
lib_tagged_allocator_copy(
  struct lib_tagged_allocator *allocator,
  void *src,
  unsigned bytes);


void
lib_tagged_allocator_free(uint64_t tag);


#ifdef __cplusplus
} /* extern */
#endif


#endif /* inc guard */
