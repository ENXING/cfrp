#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "logger.h"

static FILE *__out__ = NULL;

FILE **__log_out()
{
    if (__out__ == NULL)
    {
        __out__ = stdout;
    }
    return &__out__;
}

static logger_level __level__ =
#ifdef DEBUG
    LOGGER_DEBUG;
#elif NO_LOG
    LOGGER_OFF;
#else
    LOGGER_INFO;
#endif

logger_level *__log_level(void)
{
    return &__level__;
}

static char *__level_name(logger_level level)
{

    switch (level)
    {
    case LOGGER_DEBUG:
        return "DEBUG";
    case LOGGER_INFO:
        return "INFO";
    case LOGGER_WARNING:
        return "WARNING";
    case LOGGER_ERROR:
        return "ERROR";
    }
    return "NULL";
}

void __log(logger_level level, const char *file, const char *func, const int line, const char *tag, const char *message, ...)
{
    if (!log_available(level))
        return;
    char buff[80];
    time_t c_time;
    time(&c_time);
    strftime(buff, 80, FORMAT_DATE, localtime(&c_time));
    FILE *out = log_out();
    va_list args;
    pid_t pid = getpid();
    va_start(args, message);
    char *level_name = __level_name(level);
    tag &&fprintf(out, "%s: ", tag);
    fprintf(out, "%s %s %d ---[%s] %s", buff, level_name, pid, func, file);
    (tag && fputs(">: ", out)) || fputs(": ", out);
    vfprintf(out, message, args);
    fputc('\n', out);
    va_end(args);
}