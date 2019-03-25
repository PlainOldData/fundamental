#ifndef ASSERT_INCLUDED_2EE1722A_3152_4D39_8B20_849C8BE531EB
#define ASSERT_INCLUDED_2EE1722A_3152_4D39_8B20_849C8BE531EB


#ifdef __cplusplus
extern "C" {
#endif


void
lib_internal_assert(
	const char *expr,
	const char *file,
	unsigned line,
	const char *func);


#ifdef _WIN32
#define LIB_ASSERT_PEDANTIC(expr) if(!(expr)) { lib_internal_assert(#expr, __FILE__, __LINE__, __FUNCTION__); }
#define LIB_ASSERT(expr) if(!(expr)) { lib_internal_assert(#expr, __FILE__, __LINE__, __FUNCTION__); }
#else
#define LIB_ASSERT_PEDANTIC(expr) if(!(expr)) { __builtin_trap(); }
#define LIB_ASSERT(expr) if(!(expr)) { __builtin_trap(); }
#endif


#ifdef __cplusplus
} /* extern */
#endif


#endif /* inc guard */
