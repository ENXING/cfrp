/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2021-01-29 11:43:05
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cfrpx/include/logger.h
 */
#ifndef __LOGGER_H
#define __LOGGER_H
#include <stdio.h>

extern FILE **__log_out(void);
#define log_out() (*__log_out())

#define OFF 0
#define ERROR 1
#define WARNING 3
#define INFO 7
#define DEBUG 15
#define ALL 15

#define FORMAT_DATE "%Y-%m-%d %H:%M:%S"

typedef unsigned int logger_level;

extern logger_level *__log_level(void);

#define log_level() (*__log_level())

#define log_available(level) ((log_level() & level) == level)


extern void __log(logger_level level, const char *file, const char *func, const int line, const char *tag, const char *message, ...);

#define log_debug(message, ...) __log(DEBUG, __FILE__, __func__, __LINE__, NULL, message, ##__VA_ARGS__);
#define log_info(message, ...) __log(INFO, __FILE__, __func__, __LINE__, NULL, message, ##__VA_ARGS__);
#define log_error(message, ...) __log(ERROR, __FILE__, __func__, __LINE__, NULL, message, ##__VA_ARGS__);
#define log_warning(message, ...) __log(WARNING, __FILE__, __func__, __LINE__, NULL, message, ##__VA_ARGS__);

#define log_tag_debug(tag, message, ...) __log(DEBUG, __FILE__, __func__, __LINE__, tag, message, ##__VA_ARGS__);
#define log_tag_info(tag, message, ...) __log(INFO, __FILE__, __func__, __LINE__, tag, message, ##__VA_ARGS__);
#define log_tag_error(tag, message, ...) __log(ERROR, __FILE__, __func__, __LINE__, tag, message, ##__VA_ARGS__);
#define log_tag_warning(tag, message, ...) __log(WARNING, __FILE__, __func__, __LINE__, tag, message, ##__VA_ARGS__);

#endif