#include <lib/dir.h>
#include <lib/fundamental.h>
#include <lib/log.h>

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#elif defined(__APPLE__)
#include <unistd.h>
#include <string.h>
#include <libproc.h>
#endif

const char*
lib_exe_dir()
{
  #ifdef _WIN32
  static char buffer_path[2048] = "\0";

  if (strcmp(buffer_path, "") == 0)
  {
    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule != NULL)
    {
      GetModuleFileNameA(NULL, buffer_path, LIB_ARR_COUNT(buffer_path));

      size_t path_length = 0;
      for (size_t i = 0; i < LIB_ARR_COUNT(buffer_path); i++)
      {
        if (buffer_path[i] == '\0') {
          path_length = i;
          break;
        }
      }

      size_t last_slash_index = 0;
      for (size_t i = 0; i < path_length; i++)
      {
        size_t r_i = (path_length - 1) - i;
        if (buffer_path[r_i] == '/' || buffer_path[r_i] == '\\') {
          last_slash_index = r_i;
          break;
        }
      }

      buffer_path[last_slash_index + 1] = '\0';
    }
  }
  return buffer_path;
  #elif defined(__linux__)
	static char buffer_path[2048] = "\0";

	if(strlen(buffer_path) == 0)
	{
		char buffer[2048];
		LIB_MEM_ZERO(buffer);

		unsigned int count = readlink("/proc/self/exe", buffer, sizeof(buffer));

		if(count != (unsigned)-1)
		{
			const char *path = dirname(buffer);
			strcpy(buffer_path, path);
			strcat(buffer_path, "/");
		}
		else
		{
			LIB_LOG_ERROR("Failed to get directory");
		}
	}

	return lib_ARR_DATA(buffer_path);
	#elif defined(__APPLE__)
  static char buffer_exe_path[2048] = "\0";

  if(strlen(buffer_exe_path) == 0)
  {
    proc_pidpath(getpid(), buffer_exe_path, sizeof(buffer_exe_path));

    size_t i;
    size_t buffer_length = (size_t)strlen(buffer_exe_path);

    size_t last_slash_index = 0;
    for(i = 0; i < buffer_length; i++) {
      size_t r_i = (buffer_length - 1) - i;
      if(buffer_exe_path[r_i] == '/' || buffer_exe_path[r_i] == '\\') {
        last_slash_index = r_i;
        break;
      }
    }

    buffer_exe_path[last_slash_index + 1] = '\0';
  }

  return buffer_exe_path;
  #endif
}
