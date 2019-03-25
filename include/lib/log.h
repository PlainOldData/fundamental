/*
  Log

  Copyright: public-domain 2017 - http://unlicense.org/
*/
#ifndef LIB_LOG_INCLUDED_55713C67_54CB_4844_B789_EC11EFA4F9E6
#define LIB_LOG_INCLUDED_55713C67_54CB_4844_B789_EC11EFA4F9E6


#include <stdint.h>
#include <string.h>
#include <stdarg.h>


#ifdef __cplusplus
extern "C" {
#endif


/* ---------------------------------------------------- [ Logging Config ] -- */



/* ---------------------------------------------------- [ Logging Macros ] -- */


#define LIB_LOGGING_FILE_NAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#if defined __APPLE__ || defined __linux__
#define LIB_LOGGING_FUNC_STR __PRETTY_FUNCTION__
#else
#define LIB_LOGGING_FUNC_STR __FUNCTION__
#endif

#define LIB_LOG_NONE(prefix)
#define LIB_LOG_ONE(prefix, msg) lib_internal_log(prefix, LIB_LOGGING_FILE_NAME, LIB_LOGGING_FUNC_STR, __LINE__, msg)
#define LIB_LOG_V(prefix, msg, ...) lib_internal_log(prefix, LIB_LOGGING_FILE_NAME, LIB_LOGGING_FUNC_STR, __LINE__, msg, __VA_ARGS__)

#define LIB_LOG_GET(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg12, arg13, arg14, arg15, arg16, ...) arg16


#if defined __APPLE__ || defined __linux__
#define LIB_LOG_ARGS(...) LIB_LOG_GET(__VA_ARGS__, LIB_LOG_V, LIB_LOG_V, LIB_LOG_V, LIB_LOG_V, LIB_LOG_V, LIB_LOG_V, LIB_LOG_V, LIB_LOG_V, LIB_LOG_V, LIB_LOG_V, LIB_LOG_V, LIB_LOG_V, LIB_LOG_ONE, LIB_LOG_NONE, )
#define LIB_LOGGER(...) LIB_LOG_ARGS(__VA_ARGS__)(__VA_ARGS__)
#else
#define TEST(fmt, ...) lib_internal_log("one", "two", "three", 123, fmt, __VA_ARGS__)
#define LIB_LOGGER(...) TEST(__VA_ARGS__)
#endif


#ifndef LIB_LOG_NO_TODO
#define LIB_LOG_TODO(...)      LIB_LOGGER("[todo]", __VA_ARGS__);
#define LIB_LOG_TODO_ONCE(...) { static int err_once = 0; if(!err_once) { err_once = 1; LIB_LOG_TODO(__VA_ARGS__); } };
#else
#define LIB_LOG_TODO(...)
#define LIB_LOG_TODO_ONCE(...)
#endif

#ifndef LIB_LOG_NO_INFO
#define LIB_LOG_INFO(...)      LIB_LOGGER("[info]", __VA_ARGS__);
#define LIB_LOG_INFO_ONCE(...) { static int err_once = 0; if(!err_once) { err_once = 1; LIB_LOG_INFO(__VA_ARGS__); } };
#else
#define LIB_LOG_INFO(...)
#define LIB_LOG_INFO_ONCE(...)
#endif

#ifndef LIB_LOG_NO_WARNING
#define LIB_LOG_WARNING(...)      LIB_LOGGER("[warn]", __VA_ARGS__);
#define LIB_LOG_WARNING_ONCE(...) { static int err_once = 0; if(!err_once) { err_once = 1; LIB_LOG_WARNING(__VA_ARGS__); } };
#else
#define LIB_LOG_WARNING(...)
#define LIB_LOG_WARNING_ONCE(...)
#endif

#ifndef LIB_LOG_NO_ERROR
#define LIB_LOG_ERROR(...)      LIB_LOGGER("[err]", __VA_ARGS__);
#define LIB_LOG_ERROR_ONCE(...) { static int err_once = 0; if(!err_once) { err_once = 1; LIB_LOG_ERROR(__VA_ARGS__); } };
#else
#define LIB_LOG_ERROR(...)
#define LIB_LOG_ERROR_ONCE(...)
#endif

#ifndef LIB_LOG_NO_FATAL
#define LIB_LOG_FATAL(...)      LIB_LOGGER("[fatal]", __VA_ARGS__);
#define LIB_LOG_FATAL_ONCE(...) { static int err_once = 0; if(!err_once) { err_once = 1; LIB_LOG_FATAL(__VA_ARGS__); } };
#else
#define LIB_LOG_FATAL(...)
#define LIB_LOG_FATAL_ONCE(...)
#endif

#ifndef LIB_LOG_NO_DEPRECATED
#define LIB_LOG_DEPRECATED(...)      LIB_LOGGER("[depr]", __VA_ARGS__);
#define LIB_LOG_DEPRECATED_ONCE(...) { static int err_once = 0; if(!err_once) { err_once = 1; LIB_LOG_DEPRECATED(__VA_ARGS__); } };
#else
#define LIB_LOG_DEPRECATED(...)
#define LIB_LOG_DEPRECATED_ONCE(...)
#endif


typedef enum _lib_log_output {

  LIB_LOG_OUTPUT_CONSOLE = 1 << 0,
  LIB_LOG_OUTPUT_FILE = 1 << 1,

} lib_log_output;


void
lib_logging_set_output(const uint32_t out);


void
lib_internal_log(const char *prefix,
  const char *file,
  const char *func,
  const uint32_t line,
  const char *msg,
  ...);


void
lib_internal_log_v(const char *prefix,
  const char *file,
  const char *func,
  const uint32_t line,
  const char *msg,
  va_list args);



#ifdef __cplusplus
} /* extern */
#endif


#endif /* inc guard */

