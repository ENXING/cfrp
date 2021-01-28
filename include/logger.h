/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2021-01-28 21:38:52
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cfrpx/include/logger.h
 */
#ifndef __LOGGER_H
#define __LOGGER_H

typedef enum
{
    DEBUG,
    INFO,
    WARNING,
    ERROR
} log_level;


extern void log_log(log_level level, const char *tag, const char *message, ...);

extern void log_debug(const char *tag, const char *message, ...);

extern void log_info(const char *tag, const char *message, ...);

extern void log_warning(const char *tag, const char *mesage, ...);

extern void log_error(const char *tag, const char *message, ...);

#endif