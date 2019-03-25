#include <lib/dlib.h>


#ifdef _WIN32
#include <Windows.h>
#include <tchar.h>
#else
#include <dlfcn.h>
#endif


void*
lib_dlib_open(
  const char *name)
{
  #ifdef _WIN32
  return (void*)LoadLibrary((const TCHAR*)name);
  #else
  return dlopen(name, RTLD_LAZY);
  #endif
}


void*
lib_dlib_get_address(
  void *handle,
  const char *name)
{
  #ifdef _WIN32
  return (void*)GetProcAddress((HMODULE)handle, (LPCSTR)name);
  #else
  return dlsym(handle, name);
  #endif
}


void
lib_dlib_close(
  void *handle)
{
  #ifdef _WIN32
  FreeLibrary((HMODULE)handle);
  #else
  dlclose(handle);
  #endif
}
