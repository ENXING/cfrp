#ifndef __LIB_H__
#define __LIB_H__
#include "list.h"
#include <time.h>

#include <unistd.h>

#define LOOP for (;;)

#define __non_null__(ptr, ret)                                                                                                                       \
  if (!ptr)                                                                                                                                          \
    return ret;

#define __null__(ptr) if (!ptr)

#define cfrp_cpu sysconf(_SC_NPROCESSORS_ONLN)

#define cfrp_void_assert(cond)                                                                                                                       \
  if (cond)                                                                                                                                          \
  return

struct cfrp_time {
  time_t ctm;
};

extern void *cfrp_malloc(size_t size);

extern void *cfrp_realloc(void *ptr, size_t size);

extern void *cfrp_calloc(size_t num, size_t size);

extern int cfrp_memzero(void *ptr, size_t size);

extern int cfrp_free(void *ptr);

extern void *cfrp_memcpy(void *dest, void *src, size_t size);

extern void *cfrp_memset(void *ptr, int c, size_t size);

extern void *cfrp_memmove(void *dest, void *src, size_t size);

extern int cfrp_memcmp(void *v1, void *v2, size_t size);

extern int cfrp_strlen(char *str);

extern char *cfrp_strcpy(char *dest, char *src);

extern int cfrp_atoi(char *str);

extern long int cfrp_atol(char *str);

extern double cfrp_atof(char *str);

extern struct cfrp_time *cfrp_nowtime(struct cfrp_time *now);

extern double cfrp_difftime(struct cfrp_time *start_time, struct cfrp_time *end_time);

extern char *cfrp_formattime(char *buffer, size_t buffer_size, char *format, struct cfrp_time *ttime);

extern pid_t cfrp_getpid();

extern pid_t cfrp_waitpid(pid_t pid, int *val);

/**
 * 检查内存中所有值是否为 `\0`
 */
static inline int cfrp_memiszero(void *ptr, size_t size) {
  __non_null__(ptr, 0);
  char *chrs = (char *)ptr;
  for (size_t i = 0; i < size && ptr; i++) {
    if (chrs[i]) {
      return 0;
    }
  }
  return 1;
}

typedef struct cfrp_time ftime;
#endif