#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdlib.h>

#include "lib.h"
#include "types.h"
#include "logger.h"

void *cfrp_malloc(size_t size)
{
    return malloc(size);
}

int cfrp_zero(void *ptr, size_t size)
{
    __non_null__(ptr, C_ERROR);
    memset(ptr, '\0', size);
    return C_SUCCESS;
}

int cfrp_free(void *ptr)
{
    __non_null__(ptr, C_ERROR);
    free(ptr);
    return C_SUCCESS;
}

void *cfrp_memset(void *ptr, int c, size_t size)
{
    __non_null__(ptr, NULL);
    return memset(ptr, c, size);
}

void *cfrp_memcopy(void *dest, void *src, size_t size)
{
    __non_null__(dest, NULL);
    __non_null__(src, NULL);
    return memcpy(dest, src, size);
}

void *cfrp_memmove(void *dest, void *src, size_t size)
{
    __non_null__(dest, NULL);
    __non_null__(dest, NULL);
    return memmove(dest, src, size);
}

void *cfrp_realloc(void *ptr, size_t size)
{
    __non_null__(ptr, NULL);
    return realloc(ptr, size);
}

void *cfrp_callc(size_t num, size_t size)
{
    return calloc(num, size);
}

struct shm_table *cfrp_shmget(size_t size)
{
    shmtable_t *st = (shmtable_t *)malloc(sizeof(shmtable_t));
    __non_null__(st, NULL);
    if ((st->shid = shmget(IPC_PRIVATE, size, IPC_CREAT | SHM_EXEC | 0777)) < 0)
    {
        log_debug("sgmget failure: %s", SYS_ERROR);
        return NULL;
    }
    st->ptr = shmat(st->shid, NULL, 0);
    if (st->ptr == (void *)-1)
    {
        log_debug("shmat failure: %s", SYS_ERROR);
        cfrp_shmfree(st);
        return NULL;
    }
    return st;
}

void *cfrp_shmblock(struct shm_table *st, size_t size)
{
    __non_null__(st, NULL);
    if (st->size < size)
        return NULL;
    cfrp_zero(st->ptr, size);
    return st->ptr;
}

int cfrp_shmfree(struct shm_table *st)
{
    __non_null__(st, C_ERROR);
    return shmctl(st->shid, IPC_RMID, NULL);
}
