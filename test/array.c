

#define ARRAY_IMPL
#include <fundamental/array.h>
#include <assert.h>
#include <stdio.h>


int*
run_tests(int *values) {
        int i, count;
        
        printf("\nCap: %d", pd_array_capacity(values));
        printf("\nSize: %d", pd_array_size(values));

        /* fill */
        printf("\nFill buffer: ");
        count = 32;
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

        /* create with capacity test */
        int *values = 0;
        pd_array_create_with_capacity(values, 2);
        values = run_tests(values);
        pd_array_destroy(values);
        assert(values == 0);

        printf("\n");

        return 0;
}
