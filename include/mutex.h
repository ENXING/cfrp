#ifndef __MUTEX_H__
#define __MUTEX_H__

#include "cfrp.h"
#include "logger.h"

#define CFRP_MUTEX_UNLOCK 0
#define CFRP_MUTEX_LOCK 1

typedef void (*mutex_sync)(struct cfrp *frp, worker_t *wk);

static inline int cfrp_atomic_cpm_set(struct cfrp_lock *lock, int oldv, int newv)
{
    return __sync_bool_compare_and_swap(&lock->mutex, oldv, newv);
}

extern int cfrp_try_lock(job_t *job);

extern int cfrp_unlock(job_t *job);

extern int cfrp_mutex(struct cfrp *frp, worker_t *wk, mutex_sync sync);

#endif