

#define ARRAY_IMPL
#include <fundamental/array.h>
#include <assert.h>
#include <stdio.h>


#define TEST_COUNT 32


int*
run_tests(int *values) {
        int i, count;
        
        printf("\nCap: %d", pd_array_capacity(values));
        printf("\nSize: %d", pd_array_size(values));

        /* fill */
        printf("\nFill buffer: ");
        count = TEST_COUNT;
        for(i = 0; i < count; ++i) {
                pd_array_push(values, i);
                printf("%d, ", i);
        }
        
        /* check */
        printf("\nCheck buffer: ");
        count = pd_array_size(values);
        for(i = 0; i < count; ++i) {
                assert(values[i] == i);
                printf("%d, ", values[i]);
        }
        
        printf("\nCap: %d", pd_array_capacity(values));
        printf("\nSize: %d", pd_array_size(values));

        /* erase every second */
        printf("\nErase every second: ");
        count = pd_array_size(values) / 2;
        for(i = 0; i < count; ++i) {
                pd_array_erase(values, i + 1);
                printf("%d, ", i * 2);
        }
        
        /* check */
        printf("\nCheck erases: ");
        count = pd_array_size(values);
        for(i = 0; i < count; ++i) {
                assert(values[i] == i * 2);
                printf("%d, ", values[i]);
        }

        printf("\nCap: %d", pd_array_capacity(values));
        printf("\nSize: %d", pd_array_size(values));

        return values;
};


int
main() {
        int *values = 0;
        struct pd_array_details_desc desc;

        /* create with default */
        printf("\n\ndefault\n");
        printf("-------");
        pd_array_create(values);
        values = run_tests(values);
        pd_array_destroy(values);
        assert(values == 0);

        /* create with capacity test */
        printf("\n\nwith capacity\n"); 
        printf("-------------");
        pd_array_create_with_capacity(values, 2);
        values = run_tests(values);
        pd_array_destroy(values);
        assert(values == 0);

        /* create with buffer */
        printf("\n\nwith stack buffer\n");
        printf("-----------------");
        int some_big_buffer[TEST_COUNT * 2];
        desc.buffer = &some_big_buffer[0];
        desc.buffer_size = sizeof(some_big_buffer);
        desc.buffer_protected = 1;
        desc.alloc_fn = malloc;
        desc.free_fn = free;
        pd_array_create_with_details(values, &desc);
        values = run_tests(values);
        pd_array_destroy(values);
        assert(values == 0);
       
        /* create with buffer */
        printf("\n\nwith stack buffer and heap allocation\n");
        printf("-----------------");
        int some_small_buffer[TEST_COUNT / 2];
        desc.buffer = &some_small_buffer[0];
        desc.buffer_size = sizeof(some_small_buffer);
        desc.buffer_protected = 1;
        desc.alloc_fn = malloc;
        desc.free_fn = free;
        pd_array_create_with_details(values, &desc);
        values = run_tests(values);
        pd_array_destroy(values);
        assert(values == 0);

        printf("\n");

        return 0;
}
