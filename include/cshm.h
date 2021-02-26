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

typedef struct cfrp_shm cfrp_shm_t;
#endif