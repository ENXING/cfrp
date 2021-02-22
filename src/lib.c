#include <stdlib.h>

#include "lib.h"
#include "logger.h"
#include "types.h"

void *cfrp_malloc(size_t size) {
  return malloc(size);
}

int cfrp_zero(void *ptr, size_t size) {
  __non_null__(ptr, C_ERROR);
  memset(ptr, '\0', size);
  return C_SUCCESS;
}

int cfrp_free(void *ptr) {
  __non_null__(ptr, C_ERROR);
  free(ptr);
  return C_SUCCESS;
}

void *cfrp_memset(void *ptr, int c, size_t size) {
  __non_null__(ptr, NULL);
  return memset(ptr, c, size);
}

void *cfrp_memcpy(void *dest, void *src, size_t size) {
  __non_null__(dest, NULL);
  __non_null__(src, NULL);
  return memcpy(dest, src, size);
}

void *cfrp_memmove(void *dest, void *src, size_t size) {
  __non_null__(dest, NULL);
  __non_null__(dest, NULL);
  return memmove(dest, src, size);
}

void *cfrp_realloc(void *ptr, size_t size) {
  __non_null__(ptr, NULL);
  return realloc(ptr, size);
}

void *cfrp_callc(size_t num, size_t size) {
  return calloc(num, size);
}

int cfrp_memcmp(void *v1, void *v2, size_t size) {
  __non_null__(v1, -2);
  __non_null__(v2, -2);
  return memcmp(v1, v2, size);
}

extern struct cfrp_time *cfrp_nowtime(struct cfrp_time *now) {
  __non_null__(now, NULL);
  time(&now->ctm);
  return now;
}

extern double cfrp_difftime(struct cfrp_time *start_time, struct cfrp_time *end_time) {
  __non_null__(start_time, 0);
  __non_null__(end_time, 0);
  return difftime(end_time->ctm, start_time->ctm);
}

char *cfrp_formattime(char *buffer, size_t buffer_size, char *format, struct cfrp_time *ttime) {
  __non_null__(ttime, NULL);
  __non_null__(buffer, NULL);
  if (strftime(buffer, buffer_size, format, localtime(&ttime->ctm)) <= 0) {
    return NULL;
  }
  return buffer;
}

int cfrp_atoi(char *str) {
  __non_null__(str, C_ERROR);
  return atoi(str);
}

long int cfrp_atol(char *str) {
  __non_null__(str, C_ERROR);
  return atol(str);
}

double cfrp_atof(char *str) {
  __non_null__(str, C_ERROR);
  return atof(str);
}

int cfrp_strlen(char *str) {
  __non_null__(str, 0);
  return strlen(str);
}

char *cfrp_strcpy(char *dest, char *src) {
  __non_null__(dest, NULL);
  __non_null__(src, NULL);
  return strcpy(dest, src);
}