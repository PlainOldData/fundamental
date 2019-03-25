/*
  platform.h
  --

  Determins various platform properties that might be desired.
*/
#ifndef PLATFORM_INCLUDED_0DB1DB2D_0560_4F8D_9690_3918C0686CC7
#define PLATFORM_INCLUDED_0DB1DB2D_0560_4F8D_9690_3918C0686CC7


/* ---------------------------------------------------- [ Compiler flags ] -- */

/* compiler flags */
#ifdef __clang__
#define LIB_COMPILER_IS_CLANG 1
#define LIB_COMPILER_IS_MSVS 0
#define LIB_COMPILER_IS_GCC 0
#elif defined _MSC_VER
#define LIB_COMPILER_IS_MSVS 1
#define LIB_COMPILER_IS_CLANG 0
#define LIB_COMPILER_IS_GCC 0
#elif defined GNU
#define LIB_COMPILER_IS_GCC 1
#define LIB_COMPILER_IS_CLANG 0
#define LIB_COMPILER_IS_MSVS 0
#endif

/* os / platform flags */
#ifdef _APPLE_
#define LIB_OS_IS_APPLE 1
#define LIB_OS_IS_NIX 1
#define LIB_OS_IS_WINDOWS 0
#define LIB_OS_IS_LINUX 0
#define LIB_OS_IS_NIX 0
#elif defined _WIN32
#define LIB_OS_IS_WINDOWS 1
#define LIB_OS_IS_APPLE 0
#define LIB_OS_IS_NIX 0
#define LIB_OS_IS_LINUX 0
#elif defined __linux__
#define LIB_OS_IS_LINUX 1
#define LIB_OS_IS_NIX 1
#define LIB_OS_IS_WINDOWS 0
#define LIB_OS_IS_APPLE 0
#endif


#endif /* inc guard */
