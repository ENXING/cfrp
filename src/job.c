#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "job.h"
#include "lib.h"
#include "logger.h"
#include "nevent.h"

void cfrp_start_worker_process(struct cfrp *frp, int num, cfrp_woker_proc proc) {
  cfrp_worker_t *wk;
  cfrp_channel_t *ch;

  INIT_LIST_HEAD(&frp->wokers.list);
  INIT_LIST_HEAD(&frp->channel_event.list);

  frp->channels  = (cfrp_channel_t *)cfrp_calloc(num, sizeof(cfrp_channel_t));
  frp->woker_num = num;

  for (int slot = 0; slot < num; slot++) {
    cfrp_spawn(frp, slot, proc);
    ch = &frp->channels[slot];
    cfrp_sync_woker_channel(frp, slot, ch);
    cfrp_channel_event_add(frp, ch);
  }

  list_foreach_entry(wk, &frp->wokers.list, list) {
    log_info("woker proc running --[%d] channel %d", wk->pid, wk->chnnel->fd);
  }
}

extern void cfrp_spawn(struct cfrp *frp, int slot, cfrp_woker_proc proc) {
  cfrp_worker_t *woker;
  cfrp_epoll_t *epoll;
  cfrp_channel_t *channel;
  int fds[2], fd_1, fd_2;
  pid_t pid;

  if (cfrp_channel_open(fds) < 0) {
    log_debug("call socketpair failure. msg: %s", SYS_ERROR);
  }

  fd_1  = fds[0];
  fd_2  = fds[1];
  woker = (cfrp_worker_t *)cfrp_malloc(sizeof(cfrp_worker_t));
  epoll = cfrp_epoll_create();

  cfrp_void_assert(!woker);
  cfrp_void_assert(!epoll);
  set_noblocking(fd_1);
  set_noblocking(fd_2);

  list_add(&woker->list, &frp->wokers.list);

  channel          = frp->channels + slot;
  channel->slot    = slot;
  channel->command = CFRP_CMD_NUI;

  woker->ctx    = frp;
  woker->chnnel = channel;

  pid = fork();

  if (pid < 0) {
    log_debug("call fork failure. msg: %s", SYS_ERROR);
  } else if (pid == 0) {
    channel->fd = fd_2;
    woker->pid  = getpid();
    epoll->pid  = getpid();

    cfrp_epoll_close(frp->epoll);

    frp->epoll = epoll;

    cfrp_channel_event_add(frp, channel);
    cfrp_channel_close(fd_1);
    proc(woker);
  } else if (pid > 0) {
    channel->fd = fd_1;
    woker->pid  = pid;

    cfrp_epoll_close(epoll);
    cfrp_channel_close(fd_2);
  }
}

extern int cfrp_channel_close(int fd) {
  return close(fd);
}

extern int cfrp_channel_open(int *arr) {
  __non_null__(arr, C_ERROR);
  return socketpair(AF_UNIX, SOCK_STREAM, 0, arr);
}

extern int cfrp_channel_send(int fd, struct cfrp_channel *ch, size_t size) {
  struct msghdr msg;
  struct iovec iov[1];

#ifdef CFRP_HAVE_MSGHDR_MSG_CONTROL
  union {
    struct cmsghdr cm;
    char control[CMSG_SPACE(sizeof(int))];
  } cmsg;

  if (ch->fd == -1) {
    msg.msg_control    = NULL;
    msg.msg_controllen = 0;
  } else {
    cfrp_memzero(&cmsg, sizeof(cmsg));
    msg.msg_control    = cmsg.control;
    msg.msg_controllen = sizeof(cmsg.control);
    cmsg.cm.cmsg_len   = CMSG_LEN(sizeof(int));
    cmsg.cm.cmsg_level = SOL_SOCKET;
    cmsg.cm.cmsg_type  = SCM_RIGHTS;
    cfrp_memcpy(CMSG_DATA(&cmsg.cm), &ch->fd, sizeof(int));
  }
  msg.msg_flags = 0;
#else
  if (ch->fd == -1) {
    msg.msg_accrights     = NULL;
    msg.msg_accrights_len = 0;
  } else {
    msg.msg_accrights     = (char *)&ch->fd;
    msg.msg_accrights_len = sizeof(int);
  }
#endif
  iov[0].iov_base = ch;
  iov[0].iov_len  = size;

  msg.msg_name    = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov     = iov;
  msg.msg_iovlen  = 1;

  if (sendmsg(fd, &msg, 0) < 0) {
    log_error("channel error [%d] msg: %s", fd, SYS_ERROR);
    return C_ERROR;
  }
  return C_SUCCESS;
}

extern int cfrp_channel_recv(int fd, struct cfrp_channel *ch, size_t size) {
  struct msghdr msg;
  struct iovec iov[1];
  int len;

#ifdef CFRP_HAVE_MSGHDR_MSG_CONTROL
  union {
    struct cmsghdr cm;
    char control[CMSG_SPACE(sizeof(int))];
  } cmsg;
  msg.msg_control    = (char *)&cmsg;
  msg.msg_controllen = sizeof(cmsg);
#else
  int fd;
  msg.accrights    = &fd;
  msg.accrightslen = sizeof(int);

#endif
  msg.msg_namelen = 0;
  msg.msg_name    = NULL;
  msg.msg_iov     = iov;
  msg.msg_iovlen  = 1;
  if ((len = recvmsg(fd, &msg, size)) < 0) {
    return C_ERROR;
  }

#ifdef CFRP_HAVE_MSGHDR_MSG_CONTROL
  if (ch->command == CFRP_CMD_OPEN_CHANNEL) {
    if (cmsg.cm.cmsg_len < (socklen_t)CMSG_LEN(sizeof(int))) {
      return C_ERROR;
    } else if (cmsg.cm.cmsg_level != SOL_SOCKET || cmsg.cm.cmsg_type != SCM_RIGHTS) {
      return C_ERROR;
    }
    cfrp_memcpy(&ch->fd, CMSG_DATA(&cmsg.cm), sizeof(int));
  }
#else
  if (ch->command == CFRP_CMD_OPEN_CHANNEL) {
    if (msg.accrightslen != sizeof(int)) {
      return C_ERROR;
    }
    ch->fd = fd;
  }
#endif
  return len;
}

int cfrp_sync_woker_channel(struct cfrp *frp, int last, struct cfrp_channel *channel) {
  struct cfrp_channel *ch;
  channel->command = CFRP_CMD_OPEN_CHANNEL;
  for (int i = 0; i < last; i++) {
    ch = frp->channels + i;
    cfrp_channel_send(ch->fd, channel, sizeof(cfrp_channel_t));
  }
  channel->command = CFRP_CMD_NUI;
  return C_SUCCESS;
}

extern int cfrp_channel_event_add(struct cfrp *frp, struct cfrp_channel *ch) {
  cfrp_list_t *head = &frp->channel_event.list;
  cfrp_event_t *ev  = cfrp_malloc(sizeof(cfrp_event_t));

  __non_null__(ev, C_ERROR);

  ev->entry.channel = ch;
  ev->type          = CFRP_CHANNEL;

  list_add(&ev->list, head);

  return cfrp_epoll_add(frp->epoll, ev);
}

extern int cfrp_channel_event_del(struct cfrp *frp, struct cfrp_channel *ch) {
  cfrp_list_t *head = &frp->channel_event.list;
  cfrp_event_t *entry;

  list_foreach_entry(entry, head, list) {
    if (entry->entry.channel == ch) {
      list_del(entry->list.prev, entry->list.next);
      cfrp_epoll_del(frp->epoll, entry);
      cfrp_channel_close(ch->fd);
      cfrp_free(entry);
      return C_SUCCESS;
    }
  }

  return C_ERROR;
}