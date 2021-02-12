#ifndef __LOGGER_H
#define __LOGGER_H
#include <stdio.h>

extern FILE **__log_out(void);
#define log_out() (*__log_out())

#define LOGGER_OFF 0
#define LOGGER_ERROR 1
#define LOGGER_WARNING 3
#define LOGGER_INFO 7
#define LOGGER_DEBUG 15
#define LOGGER_ALL 15

#define FORMAT_DATE "%Y-%m-%d %H:%M:%S"

typedef unsigned int logger_level;

extern logger_level *__log_level(void);

#define log_level() (*__log_level())

#define log_available(level) ((log_level() & level) == level)

extern void __log(logger_level level, const char *file, const char *func, const int line, const char *tag, const char *message, ...);

#define log_debug(message, ...) __log(LOGGER_DEBUG, __FILE__, __func__, __LINE__, NULL, message, ##__VA_ARGS__);
#define log_info(message, ...) __log(LOGGER_INFO, __FILE__, __func__, __LINE__, NULL, message, ##__VA_ARGS__);
#define log_error(message, ...) __log(LOGGER_ERROR, __FILE__, __func__, __LINE__, NULL, message, ##__VA_ARGS__);
#define log_warning(message, ...) __log(LOGGER_WARNING, __FILE__, __func__, __LINE__, NULL, message, ##__VA_ARGS__);

#define log_tag_debug(tag, message, ...) __log(LOGGER_DEBUG, __FILE__, __func__, __LINE__, tag, message, ##__VA_ARGS__);
#define log_tag_info(tag, message, ...) __log(LOGGER_INFO, __FILE__, __func__, __LINE__, tag, message, ##__VA_ARGS__);
#define log_tag_error(tag, message, ...) __log(LOGGER_ERROR, __FILE__, __func__, __LINE__, tag, message, ##__VA_ARGS__);
#define log_tag_warning(tag, message, ...) __log(LOGGER_WARNING, __FILE__, __func__, __LINE__, tag, message, ##__VA_ARGS__);

#endif