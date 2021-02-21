#include "carray.h"
#include "lib.h"
#include "list.h"

struct array_entry {
  void *data;
  struct list_head list;
};

struct cycle_array *make_array(uint num, uint size) {
  struct cycle_array *arr = cfrp_malloc(sizeof(struct cycle_array));
  struct array_entry *entry = cfrp_malloc(sizeof(struct array_entry));
  __non_null__(arr, NULL);
  __non_null__(entry, NULL);
  INIT_LIST_HEAD(&entry->list);
  arr->num = num;
  arr->size = size;
  arr->data_ptr = cfrp_malloc(size * num);
  arr->entrys = entry;
  __non_null__(arr->data_ptr, NULL);
  return arr;
}

struct cycle_array *array_assign(void *data_ptr, uint num, uint size) {
  __non_null__(data_ptr, NULL);
  cfrp_zero(data_ptr, size * num);
  struct cycle_array *arr = cfrp_malloc(sizeof(struct cycle_array));
  struct array_entry *entry = cfrp_malloc(sizeof(struct array_entry));
  __non_null__(arr, NULL);
  __non_null__(entry, NULL);
  INIT_LIST_HEAD(&entry->list);
  arr->num = num;
  arr->size = size;
  arr->data_ptr = data_ptr;
  arr->entrys = entry;
}

int array_push(struct cycle_array *arr, void *src) {
}

int array_insert(struct cycle_array *arr, void *src, uint index) {
}

int array_remove(struct cycle_array *arr, uint index) {
}

void *array_get(struct cycle_array *arr, uint index) {
}

void *array_find(struct cycle_array *arr, void *dest, array_find_handler find) {
  for (size_t i = 0; i < arr->num; i++) {
  }
}