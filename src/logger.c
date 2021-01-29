/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2021-01-29 11:41:01
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cfrpx/src/logger.c
 */
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "logger.h"

FILE *__out__ = NULL;

FILE **__log_out()
{
    if (__out__ == NULL)
    {
        __out__ = stdout;
    }
    return &__out__;
}

logger_level __level__ = INFO;

logger_level *__log_level(void)
{
    return &__level__;
}

char *__level_name(logger_level level)
{

    switch (level)
    {
    case DEBUG:
        return "DEBUG";
    case INFO:
        return "INFO";
    case WARNING:
        return "WARNING";
    case ERROR:
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