#include <unistd.h>

#include "job.h"
#include "lib.h"
#include "logger.h"
#include "nevent.h"

void cfrp_start_worker_process(struct cfrp *frp, woker_handler run_process) {
  pid_t mpid = getpid();
  pid_t pid;
  fctx_t *ctx = (fctx_t *)frp->ctx;
  for (int i = 0; i < cfrp_cpu; i++) {
    if ((pid = fork()) < 0) {
      log_error("cfrp woker process error! %s", SYS_ERROR);
    } else if ((pid = getpid()) != mpid) {
      break;
    }
  }
  if (pid == mpid) {
    return;
  } else {
    fworker_t *wk = (fworker_t *)cfrp_malloc(sizeof(fworker_t));
    struct cfrp_epoll *epoll = cfrp_epoll_create();
    __non_null__(wk, ;);
    __non_null__(epoll, ;);
    wk->ctx = frp;
    wk->pid = pid;
    wk->epoll = epoll;
    run_process(wk);
  }
}