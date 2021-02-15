#include "mutex.h"
#include "unistd.h"
#include "logger.h"

int cfrp_try_lock(job_t *job)
{
    return (job->lock->mutex == CFRP_MUTEX_UNLOCK && cfrp_atomic_cpm_set(job->lock, CFRP_MUTEX_UNLOCK, CFRP_MUTEX_LOCK));
}

int cfrp_unlock(job_t *job)
{
    return cfrp_atomic_cpm_set(job->lock, CFRP_MUTEX_LOCK, CFRP_MUTEX_UNLOCK);
}

int cfrp_mutex(struct cfrp *frp, worker_t *wk, mutex_sync sync)
{
    if (cfrp_try_lock(&frp->job))
    {
        log_debug("cfrp locked. pid: %d", wk->pid);
        sync(frp, wk);
        cfrp_unlock(&frp->job);
        log_debug("cfrp unlock. pid: %d", wk->pid);
    }
}