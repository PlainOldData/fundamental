#include <lib/assert.h>
#include <lib/fundamental.h>

#include <assert.h>
#ifdef _WIN32
#include <assert.h>
#include <windows.h>
#endif

#include <stdlib.h>


void
lib_internal_assert(
	const char *expr,
	const char *file,
	unsigned line,
	const char *func)
{
  #ifdef _WIN32
  LIB_UNUSED(func);

  wchar_t wmsg[2024];
  MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, expr, -1, wmsg, LIB_ARR_COUNT(wmsg));

  wchar_t wfile[2024];
  MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, file, -1, wfile, LIB_ARR_COUNT(wfile));

  _wassert(wmsg, wfile, line);
  #else
	LIB_UNUSED(expr);
	LIB_UNUSED(file);
	LIB_UNUSED(line);
	LIB_UNUSED(func);
	/*__assert_fail (expr, file, line, func);*/
  abort();
  #endif
}	
