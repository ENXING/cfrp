#ifndef __ARRAY_H__
#define __ARRAY_H__

#include "types.h"

struct cycle_array {
  uint size;
  uint num;
  uint pos;
  void *data_ptr;
  void *entrys;
};

typedef int (*array_find_handler)(void *dest, void *src);

extern struct cycle_array *make_array(uint num, uint size);

extern struct cycle_array *array_assign(void *data_ptr, uint num, uint size);

extern int array_push(struct cycle_array *arr, void *src);

extern int array_insert(struct cycle_array *arr, void *src, uint index);

extern int array_remove(struct cycle_array *arr, uint index);

extern void *array_get(struct cycle_array *arr, uint index);

extern void *array_find(struct cycle_array *arr, void *dest, array_find_handler find);

#endif