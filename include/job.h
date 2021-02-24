#ifndef __JOB_H__
#define __JOB_H__
#include <stdlib.h>
#include <unistd.h>

#include "cfrp.h"

#define CFRP_CMD_NUI -1
#define CFRP_CMD_OPEN_CHANNEL 1
#define CFRP_CMD_EXIT 2

#define CFRP_HAVE_MSGHDR_MSG_CONTROL

typedef void (*cfrp_woker_proc)(cfrp_worker_t *wk);

extern void cfrp_start_worker_process(struct cfrp *frp, int num, cfrp_woker_proc proc);

extern void cfrp_spawn(struct cfrp *frp, int i, cfrp_woker_proc proc);

extern int cfrp_channel_close(int fd);

extern int cfrp_channel_open(int *arr);

/**
 * 将子进程间都的通道都同步
 */
extern int cfrp_sync_woker_channel(struct cfrp *frp, int last, struct cfrp_channel *channel);

extern int cfrp_channel_event_add(struct cfrp *frp, struct cfrp_channel *ch);

extern int cfrp_channel_event_del(struct cfrp *frp, struct cfrp_channel *ch);

extern int cfrp_channel_send(int fd, struct cfrp_channel *ch, size_t size);

extern int cfrp_channel_recv(int fd, struct cfrp_channel *ch, size_t size);

#endif