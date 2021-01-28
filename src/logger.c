/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2021-01-28 22:07:42
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cfrpx/src/logger.c
 */
#include <stdarg.h>
#include <stdio.h>
#include "logger.h"

void log_log(log_level level, const char *tag, const char *message, ...)
{
    printf("====\n");
}


void log_debug(const char *tag, const char *message, ...)
{
    log_log(DEBUG, tag, message);
}

void log_info(const char *tag, const char *message, ...)
{
    log_log(INFO, tag, message);
}

void log_warning(const char *tag, const char *mesage, ...)
{
    log_log(WARNING, tag, mesage);
}

void log_error(const char *tag, const char *message, ...)
{
    log_log(ERROR, tag, message);
}