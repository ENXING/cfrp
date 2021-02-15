#ifndef __LIB_H__
#define __LIB_H__
#include <unistd.h>
#include "list.h"

#define LOOP for (;;)

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

extern struct shm_table *cfrp_shmget(size_t size);

extern void *cfrp_shmblock(struct shm_table *st, size_t size);

extern int cfrp_shmfree(struct shm_table *tab);

typedef struct shm_table shmtable_t;

#endif