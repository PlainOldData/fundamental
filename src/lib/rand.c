#include <lib/rand.h>


unsigned
lib_rand_xorshift_with_seed(
        unsigned seed)
{
        seed ^= seed << 13;
        seed ^= seed >> 17;
        seed ^= seed << 15;
        return seed;
}


unsigned
lib_rand_xorshift()
{
        static unsigned rand_seed = 1;
        unsigned x = rand_seed;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 15;
        rand_seed = x;

        return x;
}


float
lib_rand_range_float(
        float start,
        float end)
{
        unsigned rand = lib_rand_xorshift();
        unsigned rand_max = ((unsigned)-1);

        return start + (float)rand / ((float)(rand_max / (end - start)));
}

