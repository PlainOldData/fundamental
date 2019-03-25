#ifndef RAND_INCLUDED_94597321_D8B4_46AB_97B7_ABD429E9E2D4
#define RAND_INCLUDED_94597321_D8B4_46AB_97B7_ABD429E9E2D4


#ifdef __cplusplus
extern "C" {
#endif


unsigned
lib_rand_xorshift_with_seed(unsigned seed);


unsigned
lib_rand_xorshift();


float
lib_rand_range_float(float start, float end);


#ifdef __cplusplus
} /* extern */
#endif


#endif /* inc guard */

