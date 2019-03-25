#include <lib/alloc.h>
#include <lib/fundamental.h>
#include <lib/array.h>
#include <lib/assert.h>
#include <lib/mutex.h>
#include <lib/atomic.h>
#include <lib/hash.h>
#include <stdlib.h>
#include <string.h>


/* ------------------------------------------------------ [ Alloc Config ] -- */


#ifndef LIB_ALLOC_STATS
#define LIB_ALLOC_STATS 1
#endif


#if LIB_ALLOC_STATS
/*#include <lib_lib/atomic.h>*/

unsigned g_alloc_counter = 0;
unsigned g_free_counter = 0;
#endif


/* --------------------------------------------------- [ Allocator State ] -- */

#define LIB_TAGGED_ID_UNOWNED 0
#define LIB_TAGGED_ID_ORPHENED -1

#define LIB_TAGGED_POOL_SIZE 2048
#define LIB_TAGGED_POOL_ALIGN 16

/* 2 MB */
#define LIB_TAGGED_PAGE_SIZE 2097152


struct lib_tagged_alloc_id {
        lib_mutex mutex;
        int pool_id;
};

struct lib_tagged_pool {
        uint64_t tagged_id;
        lib_atomic_int owned_by;
        void *block;
        unsigned bytes_left;
};


struct lib_allocator_ctx {
        lib_mutex mutex;
  
        lib_atomic_int tagged_alloc_status[128];
        struct lib_tagged_alloc_id tagged_allocators[128];
        struct lib_tagged_pool tagged_pools[256];
};


static struct lib_allocator_ctx alloc_ctx = {0};


/* -------------------------------------------------- [ Tagged Allocator ] -- */


static void
internal_tagged_new_pool(
        int owner_index,
        uint64_t cat_id,
        struct lib_tagged_pool **pool,
        int *pool_id)
{
        int i;
        int count = LIB_ARR_COUNT(alloc_ctx.tagged_pools);

        if (*pool) {
                lib_atomic_int_store(&(*pool)->owned_by, LIB_TAGGED_ID_ORPHENED);
        }

        for (i = 0; i < count; ++i) {
                struct lib_tagged_pool *new_pool = &alloc_ctx.tagged_pools[i];

                int old_owner = lib_atomic_int_compare_and_swap(
                        &new_pool->owned_by,
                        LIB_TAGGED_ID_UNOWNED,
                        owner_index);

                if (old_owner == LIB_TAGGED_ID_UNOWNED) {
                        *pool = new_pool;
                        (*pool)->bytes_left = LIB_TAGGED_PAGE_SIZE;
                        (*pool)->tagged_id = cat_id;

                        if ((*pool)->block == 0) {
                                (*pool)->block = malloc(LIB_TAGGED_PAGE_SIZE);
                        }

                        *pool_id = i;

                        break;
                }
        }
}

void*
tagged_alloc(
        struct lib_allocator *allocator,
        unsigned bytes)
{
        unsigned bytes_required = bytes + LIB_TAGGED_POOL_ALIGN;
  
        if(bytes_required > LIB_TAGGED_PAGE_SIZE) {
                LIB_ASSERT(0);
                return 0;
        }
  
        uint32_t index = (uint32_t)LIB_LOWER_32_BITS(allocator->alloc_id);
        struct lib_tagged_alloc_id *alloc = &alloc_ctx.tagged_allocators[index];
  
        lib_mutex_lock(alloc->mutex);
  
        int pool_id = alloc->pool_id;
  
        struct lib_tagged_pool *pool = NULL;
        
        if (pool_id != LIB_TAGGED_ID_ORPHENED) {
                pool = &alloc_ctx.tagged_pools[pool_id];
        }

        /* first time used allocate pool */
        if(pool_id == -1 || pool->bytes_left < bytes_required) {
                internal_tagged_new_pool(index, allocator->category_id, &pool, &pool_id);

                alloc->pool_id = pool_id;
        }
  
        /* allocate */
        /* need to align */
        unsigned char *block = (unsigned char *)pool->block;

        void * addr = (void*)&block[LIB_TAGGED_PAGE_SIZE - pool->bytes_left];
  
        pool->bytes_left -= bytes_required;
  
        lib_mutex_unlock(alloc->mutex);

        return addr;
}


void*
tagged_zalloc(
        struct lib_allocator *allocator,
        unsigned bytes)
{
        unsigned char *addr = (unsigned char *)tagged_alloc(allocator, bytes);

        unsigned i;

        for (i = 0; i < bytes; ++i) {
               addr[i] = 0;
        }
  
        return (void*)addr;
}


void
tagged_free(
        struct lib_allocator *allocator,
        void *addr)
{
}


/* ------------------------------------------------- [ M<naual Allocator ] -- */


void*
std_alloc(
        struct lib_allocator *allocator,
        unsigned bytes)
{
        (void)allocator;
        return malloc(bytes);
}


void*
std_zalloc(
        struct lib_allocator *allocator,
        unsigned bytes)
{
        (void)allocator;
        return calloc(bytes, 1);
}


void
std_free(
        struct lib_allocator *allocator,
        void *addr)
{
        (void)allocator;
        free(addr);
}


/* --------------------------------------------------------- [ Allocator ] -- */


LIB_STATUS
lib_allocator_create(
        struct lib_allocator_create_desc *desc,
        struct lib_allocator *allocator,
        int count)
{
        int i;

        /* check param exist */
        if(LIB_PEDANTIC && (!desc || !allocator || !count)) {
                return LIB_INVALID_PARAMS;
        }
  
        /* check some param requirements */
        if(LIB_PEDANTIC) {
                for(i = 0; i < count; ++i) {
                        const char *cat_name = desc[i].category_name;
                        if(!cat_name || !strlen(cat_name)) {
                                return LIB_INVALID_PARAMS;
                        }
                }
        }
  
        /* create allocators */
        for(i = 0; i < count; ++i) {
                uint64_t hash_id = lib_hash(desc->category_name);

                if(desc->type == LIB_ALLOCATOR_MANUAL) {
                        uint64_t alloc_id = LIB_PACK3232(0, (uint32_t)desc->type);

                        allocator[i].alloc_id = alloc_id;
                        allocator[i].category_id = hash_id;
                }
                else if(desc->type == LIB_ALLOCATOR_TAGGED) {
                        /* find an allocator */
                        int alloc_count = LIB_ARR_COUNT(alloc_ctx.tagged_alloc_status);
                        int j;
                        
                        /* start at one because static init means owner_id is zero */
                        for(j = 1; j < alloc_count; ++j) {
                                int status = lib_atomic_int_compare_and_swap(&alloc_ctx.tagged_alloc_status[j], 0, 1);
                          
                                if(status != 0) {
                                        continue;
                                }
                          
                                alloc_ctx.tagged_allocators[j].pool_id = -1;
                          
                                if(alloc_ctx.tagged_allocators[j].mutex == 0) {
                                        alloc_ctx.tagged_allocators[j].mutex = lib_mutex_create();
                                }
                          
                                break;
                        }

                        uint32_t index = j;
                        uint32_t type_id = (uint32_t)desc->type;
                  
                        uint64_t alloc_id = LIB_PACK3232(index, type_id);

                        allocator[i].alloc_id = alloc_id;
                        allocator[i].category_id = hash_id;
                }
        }
  
        return LIB_SUCCESS;
 }


LIB_STATUS
lib_allocator_destroy(
        struct lib_allocator *allocator)
{
        //LIB_ASSERT(0);
        return LIB_ERROR;
}


LIB_STATUS
lib_allocator_tagged_get_data(
        struct lib_allocator_tagged_data *data)
{
        LIB_ASSERT(data);
        LIB_ASSERT(data->type_id == LIB_ALLOCATOR_STRUCT_TAGGED_DATA);
        
        data->total_pools = LIB_ARR_COUNT(alloc_ctx.tagged_pools);
        data->in_use_pools = 0;
        data->allocated_pools = 0;
        
        int i;
        int count = LIB_ARR_COUNT(alloc_ctx.tagged_pools);

        for (i = 0; i < count; ++i) {
                if (alloc_ctx.tagged_pools[i].block) {
                        data->allocated_pools += 1;

                        int owner_id = lib_atomic_int_load(
                                &alloc_ctx.tagged_pools[i].owned_by);

                        if (owner_id != 0) {
                                data->in_use_pools += 1;
                        }
                }
        }

        return LIB_SUCCESS;
}


LIB_STATUS
lib_allocator_tagged_clear_category(
        struct lib_allocator_tagged_clear *clear)
{
        LIB_ASSERT(clear);
        LIB_ASSERT(clear->type_id == LIB_ALLOCATOR_STRUCT_TAGGED_CLEAR);
        LIB_ASSERT(clear->category_to_clear);
        LIB_ASSERT(strlen(clear->category_to_clear));

        uint64_t hash_id = lib_hash(clear->category_to_clear);



        int i;
        int count = LIB_ARR_COUNT(alloc_ctx.tagged_pools);

        for (i = 0; i < count; ++i) {
                struct lib_tagged_pool *pool = &alloc_ctx.tagged_pools[i];

                if (pool->tagged_id == hash_id) {
                        pool->bytes_left = LIB_TAGGED_PAGE_SIZE;

                        lib_atomic_int_compare_and_swap(
                                &pool->owned_by,
                                LIB_TAGGED_ID_ORPHENED,
                                LIB_TAGGED_ID_UNOWNED);
                }
        }

        return LIB_SUCCESS;
}


void*
lib_alloc(struct lib_allocator *allocator, unsigned bytes)
{
        uint32_t type = LIB_UPPER_32_BITS(allocator->alloc_id);

        if(type == LIB_ALLOCATOR_MANUAL) {
                return std_alloc(allocator, bytes);
        }
        else if (type == LIB_ALLOCATOR_TAGGED) {
                return tagged_alloc(allocator, bytes);
        }

        return 0;
}


void*
lib_zalloc(struct lib_allocator *allocator, unsigned bytes)
{
        uint32_t type = LIB_UPPER_32_BITS(allocator->alloc_id);

        if (type == LIB_ALLOCATOR_MANUAL) {
                return std_zalloc(allocator, bytes);
        }
        else if (type == LIB_ALLOCATOR_TAGGED) {
                return tagged_zalloc(allocator, bytes);
        }

        return 0;
}


void
lib_free(struct lib_allocator *allocator, void *addr)
{
        uint32_t type = LIB_UPPER_32_BITS(allocator->alloc_id);

        if(type == LIB_ALLOCATOR_MANUAL) {
                std_free(allocator, addr);
        }
        else if (type == LIB_ALLOCATOR_TAGGED) {
                tagged_free(allocator, addr);
        }
}


/* ------------------------------------------------------ [ Alloc Config ] -- */


#undef LIB_ALLOC_STATS
