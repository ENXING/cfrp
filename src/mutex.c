#include "mutex.h"
#include "lib.h"
#include "logger.h"

int cfrp_try_lock(cfrp_lock_t *lock) {
  __non_null__(lock, C_ERROR);
  return (lock->mutex == CFRP_MUTEX_UNLOCK && cfrp_atomic_cpm_set(lock, CFRP_MUTEX_UNLOCK, CFRP_MUTEX_LOCK));
}

int cfrp_unlock(cfrp_lock_t *lock) {
  __non_null__(lock, C_ERROR);
  return cfrp_atomic_cpm_set(lock, CFRP_MUTEX_LOCK, CFRP_MUTEX_UNLOCK);
}

void cfrp_mutex(struct cfrp *frp, cfrp_worker_t *wk, mutex_sync sync_exec) {
  __non_null__(frp, ;);
  __non_null__(wk, ;);
  if (cfrp_try_lock(frp->lock)) {
    log_debug("cfrp acquire %d", wk->pid);
    ftime start_tm, end_tm;
    cfrp_nowtime(&start_tm);
    frp->lock->aptr = wk;
    sync_exec(frp, wk);
    cfrp_nowtime(&end_tm);
    log_debug("execute time consuming: %2f seconds", cfrp_difftime(&start_tm, &end_tm));
    cfrp_unlock(frp->lock);
    log_debug("cfrp release %d", wk->pid);
  }
}