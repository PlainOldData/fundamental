#include <stdio.h>
#include <lib/log.h>
#include <lib/fundamental.h>
#include <lib/dir.h>


uint32_t logging_outputs = LIB_LOG_OUTPUT_CONSOLE;
char filepath[2048];


void
lib_logging_set_output(const uint32_t output)
{
  logging_outputs = output;

  if (logging_outputs & LIB_LOG_OUTPUT_FILE)
  {
    memset(filepath, 0, sizeof(filepath));

    strcat(filepath, lib_exe_dir());
    strcat(filepath, "logging");

    #ifndef NDEBUG
    strcat(filepath, "-debug");
    #else
    strcat(filepath, "-release");
    #endif

    strcat(filepath, ".txt");

    FILE *log_file = fopen(filepath, "wb");

    if (log_file)
    {
      char log[] = "Logging\n--\n\n";

      fwrite(log, sizeof(char), sizeof(log), log_file);
      fclose(log_file);
    }
  }
}


void
lib_internal_log(const char *prefix,
  const char *file,
  const char *func,
  const uint32_t line,
  const char *msg,
  ...)
{
  va_list args;
  va_start(args, msg);
  lib_internal_log_v(prefix, file, func, line, msg, args);
  va_end(args);
}


void
lib_internal_log_v(const char *prefix,
  const char *file,
  const char *func,
  const uint32_t line,
  const char *msg,
  va_list args)
{
  if (logging_outputs & LIB_LOG_OUTPUT_CONSOLE)
  {
    printf("\n-\n");

    if (prefix) { printf("%s ", prefix); }
    if (file && line) { printf("%s : %s : %d ", file, func, line); }

    if ((prefix || (file || func)) && msg)
    {
      printf("\n");
    }

    if (msg) { vprintf(msg, args); }

    if (prefix || (file && line) || msg)
    {
      printf("\n-\n");
    }
  }

  if (logging_outputs & LIB_LOG_OUTPUT_FILE)
  {
    size_t buffer_used = 5; /* start at 5 to account for trailing characters. */

    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));

    if (prefix)
    {
      buffer_used += strlen(prefix) + 1;

      if (buffer_used < 2048)
      {
        strcat(buffer, prefix);
        strcat(buffer, "\n");
      }
    }

    if (msg)
    {
      buffer_used += strlen(msg) + 1;

      if (buffer_used < 2048)
      {
        strcat(buffer, msg);
        strcat(buffer, "\n");
      }
    }

    strcat(buffer, "-\n");

    FILE *log_file = fopen(filepath, "a+");

    if (log_file)
    {
      fwrite((void*)buffer, sizeof(char), sizeof(char) * strlen(buffer), log_file);
      fclose(log_file);
    }
  }
}
