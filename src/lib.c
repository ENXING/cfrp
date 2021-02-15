#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdlib.h>

#include "lib.h"
#include "types.h"
#include "logger.h"

struct shm_table *cfrp_shmget(size_t size)
{
    shmtable_t *st = (shmtable_t *)malloc(sizeof(shmtable_t));
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
    if (st->size < size)
        return NULL;
    memset(st->ptr, '\0', size);
    return st->ptr;
}

int cfrp_shmfree(struct shm_table *st)
{
    shmctl(st->shid, IPC_RMID, NULL);
}
