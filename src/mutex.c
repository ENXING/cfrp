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

void cfrp_force_lock(cfrp_lock_t *lock) {
  do {
    cfrp_unlock(lock);
  } while (!cfrp_try_lock(lock));
}

void cfrp_mutex(struct cfrp *frp, mutex_sync sync_exec) {
  __non_null__(frp, ;);

  if (cfrp_try_lock(frp->lock)) {

    log_debug("cfrp acquire %s", frp->name);

    ftime start_tm, end_tm;

    cfrp_nowtime(&start_tm);

    frp->lock->aptr = frp;

    sync_exec(frp);

    cfrp_nowtime(&end_tm);

    log_debug("execute time consuming: %2f seconds", cfrp_difftime(&start_tm, &end_tm));

    cfrp_unlock(frp->lock);

    log_debug("cfrp release %s", frp->name);
  }
}