#ifndef __JOB_H__
#define __JOB_H__
#include "cfrp.h"
#include <stdlib.h>
#include <unistd.h>

typedef void (*woker_handler)(fworker_t *wk);

extern void cfrp_start_worker_process(struct cfrp *frp,
                                      woker_handler run_process);

#endif