#ifndef __LIB_H__
#define __LIB_H__
#include <unistd.h>
#include "list.h"

#define LOOP for (;;)

#define __non_null__(ptr, ret) \
    if (!ptr)                  \
        return ret;

#define __null__(ptr) if (!ptr)

#define cfrp_cpu sysconf(_SC_NPROCESSORS_ONLN)

struct shm_block
{
    size_t size;
    void *ptr;
    struct list_head *head;
    struct list_head list;
};

struct shm_table
{
    int shid;
    size_t size;
    void *ptr;
    struct shm_block blocks;
};

extern void *cfrp_malloc(size_t size);

extern void *cfrp_realloc(void *ptr, size_t size);

extern void *cfrp_calloc(size_t num, size_t size);

extern int cfrp_zero(void *ptr, size_t size);

extern int cfrp_free(void *ptr);

extern void *cfrp_memcopy(void *dest, void *src, size_t size);

extern void *cfrp_memset(void *ptr, int c, size_t size);

extern void *cfrp_memmove(void *dest, void *src, size_t size);

extern struct shm_table *cfrp_shmget(size_t size);

extern void *cfrp_shmblock(struct shm_table *st, size_t size);

extern int cfrp_shmfree(struct shm_table *tab);

typedef struct shm_table shmtable_t;

#endif