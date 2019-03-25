#ifndef FUNDAMENTAL_INCLUDED_C3B2733E_2BD0_47C9_97D8_1EEDD88B1AB5
#define FUNDAMENTAL_INCLUDED_C3B2733E_2BD0_47C9_97D8_1EEDD88B1AB5


/* ---------------------------------------------------- [ fixed integers ] -- */


#ifndef __cplusplus
	#ifdef _MSC_VER
		typedef signed   __int8  int8_t;
		typedef unsigned __int8  uint8_t;
		typedef signed   __int16 int16_t;
		typedef unsigned __int16 uint16_t;
		typedef signed   __int32 int32_t;
		typedef unsigned __int32 uint32_t;
		typedef signed   __int64 int64_t;
		typedef unsigned __int64 uint64_t;
		#include <stddef.h>
	#else
		#include <stdint.h>
		#include <stddef.h>
	#endif
#else
	#include <cstdint>
	#include <cstddef>
#endif


/* --------------------------------------------------------- [ lib flags ] -- */


#ifdef NDEBUG
#define LIB_DEBUG 0
#define LIB_PEDANTIC 1
#else
#define LIB_DEBUG 1
#define LIB_PEDANTIC 1
#endif


/* ----------------------------------------------- [ generic error codes ] -- */


typedef int LIB_STATUS;

#define LIB_SUCCESS 1
#define LIB_EMPTY 0
#define LIB_ERROR -1
#define LIB_INVALID_PARAMS -2
#define LIB_NOT_READY -3


/* ------------------------------------------------------- [ basic types ] -- */


#define LIB_NULL 0

typedef int LIB_BOOL;
#define LIB_TRUE 1
#define LIB_FALSE 0


/* ----------------------------------------------------- [ array helpers ] -- */


#define LIB_ARR_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))
#define LIB_ARR_DATA(arr) (&arr[0])


/* ---------------------------------------------------- [ config helpers ] -- */


#define LIB_IS_ENABLED(expr) ((expr) != 0)


/* --------------------------------------------------------- [ min / max ] -- */


#define LIB_MIN(a, b) (a < b ? a : b)
#define LIB_MAX(a, b) (a > b ? a : b)


/* ------------------------------------------------------- [ conversions ] -- */


#define LIB_MB_TO_BYTES(mb) (mb * 1048576)


/* -------------------------------------------------------------- [ bits ] -- */


#define LIB_BIT(ui) 1 << ui

#define LIB_UPPER_32_BITS(ui64) (ui64 & 0xFFFFFFFF)
#define LIB_LOWER_32_BITS(ui64) (ui64 >> 32)
#define LIB_FIRST8_BITS(ui32) (ui32 >> 24 & 0xFF)
#define LIB_SECOND8_BITS(ui32) (ui32 >> 16 & 0xFF)
#define LIB_THIRD8_BITS(ui32) (ui32 >> 8 & 0xFF)
#define LIB_FORTH8_BITS(ui32) (ui32 >> & 0xFFFFFF)
#define LIB_UPPER_24_BITS(ui32) (ui32 & 0xFFFFFF)

#define LIB_PACK1616(ui16a, ui16b) (((uint32_t)ui16a) << 16 | ui16b)
#define LIB_PACK3232(ui32a, ui32b) (((uint64_t)ui32a) << 32 | ui32b)
#define LIB_PACK8888(ui8a, ui8b, ui8c, ui8d) (uint32_t)((uint32_t)ui8a << 24) | ((uint32_t)ui8b << 16) | ((uint32_t)ui8c << 8) | ((uint32_t)ui8d << 0))
#define LIB_PACK824(ui8a, ui32b) (uint32_t)(ui8a << 24) | ui32b


/* ------------------------------------------------------ [ util helpers ] -- */


#define LIB_UNUSED(x) (void)(x)
#define LIB_MEM_ZERO(var) do { unsigned _i; for(_i = 0; _i < sizeof(var); ++_i) {((unsigned char *) &var)[_i] = 0;} } while(0)
#define LIB_MEM_CPY(dst, src) do { unsigned _i; for(_i = 0; _i < sizeof(src); ++_i) { ((unsigned char*) dst)[_i] = ((unsigned char*) src)[_i]; } } while(0)
#define LIB_STRINGIFY(a) _LIB_STRINGIFY(a)
#define _LIB_STRINGIFY(a) #a

#endif /* inc guard */
