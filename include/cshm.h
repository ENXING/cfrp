#ifndef __CSHM_H__
#define __CSHM_H__

#include "list.h"
#include "types.h"

struct cfrp_shm {
  size_t size;
  void *entry;
};

extern struct cfrp_shm *make_cshm(size_t size);

extern void *cshm_alloc(struct cfrp_shm *shm, size_t size);

extern void cshm_release(struct cfrp_shm *shm);

static inline int cshm_isnui(void *ptr, size_t size) {
  char *chrs = (char *)ptr;
  for (size_t i = 0; i < size && ptr; i++) {
    if (chrs[i]) {
      return 0;
    }
  }
  return 1;
}

typedef struct cfrp_shm cfrp_shm_t;
#endif