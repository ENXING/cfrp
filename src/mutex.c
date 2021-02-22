#include "mutex.h"
#include "lib.h"
#include "logger.h"

int cfrp_try_lock(fjob_t *job) {
  __non_null__(job, C_ERROR);
  __non_null__(job->lock, C_ERROR);
  return (job->lock->mutex == CFRP_MUTEX_UNLOCK && cfrp_atomic_cpm_set(job->lock, CFRP_MUTEX_UNLOCK, CFRP_MUTEX_LOCK));
}

int cfrp_unlock(fjob_t *job) {
  __non_null__(job, C_ERROR);
  __non_null__(job->lock, C_ERROR);
  return cfrp_atomic_cpm_set(job->lock, CFRP_MUTEX_LOCK, CFRP_MUTEX_UNLOCK);
}

void cfrp_mutex(struct cfrp *frp, fworker_t *wk, mutex_sync sync_exec) {
  __non_null__(frp, ;);
  __non_null__(wk, ;);
  if (cfrp_try_lock(&frp->job)) {
    log_debug("cfrp acquire %d", wk->pid);
    ftime start_tm, end_tm;
    cfrp_nowtime(&start_tm);
    frp->job.lock->aptr = wk;
    sync_exec(frp, wk);
    cfrp_nowtime(&end_tm);
    log_debug("execute time consuming: %2f seconds", cfrp_difftime(&start_tm, &end_tm));
    cfrp_unlock(&frp->job);
    log_debug("cfrp release %d", wk->pid);
  }
}