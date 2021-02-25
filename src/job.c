#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "channel.h"
#include "job.h"
#include "lib.h"
#include "logger.h"
#include "nevent.h"

void cfrp_start_worker_process(struct cfrp *frp, int num, cfrp_woker_proc_t proc, cfrp_complete_t cfrp_complete) {
  cfrp_worker_t *wk;
  cfrp_channel_t *ch;

  int slot;

  INIT_LIST_HEAD(&frp->workers.list);
  INIT_LIST_HEAD(&frp->channel_event.list);

  frp->channels   = (cfrp_channel_t *)cfrp_calloc(num, sizeof(cfrp_channel_t));
  frp->worker_num = num;

  for (slot = 0; slot < num; slot++) {
    cfrp_spawn(frp, slot, proc);
    ch = &frp->channels[slot];
    cfrp_sync_woker_channel(frp, slot, ch);
    cfrp_channel_event_add(frp, ch);
  }

  if (cfrp_complete) {
    cfrp_complete(frp);
  }
}

extern void cfrp_spawn(struct cfrp *frp, int slot, cfrp_woker_proc_t woker_proc) {
  cfrp_worker_t *worker;
  cfrp_epoll_t *epoll;
  cfrp_channel_t *channel;
  int fds[2], fd_1, fd_2;
  pid_t pid;

  if (cfrp_channel_open(fds) < 0) {
    log_debug("call socketpair failure. msg: %s", CFRP_SYS_ERROR);
  }

  fd_1   = fds[0];
  fd_2   = fds[1];
  worker = (cfrp_worker_t *)cfrp_malloc(sizeof(cfrp_worker_t));
  epoll  = cfrp_epoll_create();

  cfrp_void_assert(!worker);
  cfrp_void_assert(!epoll);
  set_noblocking(fd_1);
  set_noblocking(fd_2);

  list_add(&worker->list, &frp->workers.list);

  channel              = &frp->channels[slot];
  channel->slot        = slot;
  channel->cmd         = CFRP_CMD_NUI;
  worker->ctx          = frp;
  worker->channel_solt = slot;
  pid                  = fork();

  if (pid < 0) {
    log_debug("call fork failure. msg: %s", CFRP_SYS_ERROR);
  } else if (pid == 0) {

    channel->fd  = fd_2;
    channel->pid = getpid();
    worker->pid  = getpid();
    epoll->pid   = getpid();

    cfrp_epoll_close(frp->epoll_private);

    frp->worker           = worker;
    frp->channel          = channel;
    frp->epoll_private    = epoll;
    frp->epoll_share->pid = getpid();

    cfrp_channel_close(fd_1);
    cfrp_channel_event_add(frp, channel);

    woker_proc(frp);

  } else if (pid > 0) {

    channel->fd  = fd_1;
    channel->pid = pid;
    worker->pid  = pid;

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

int cfrp_sync_woker_channel(struct cfrp *frp, int last, struct cfrp_channel *channel) {

  cfrp_channel_t *ch;

  cfrp_cmsg_t msg = {.cmd = CFRP_CMD_OPEN_CHANNEL, .fd = channel->fd};

  cfrp_memcpy(&msg.data.channel, channel, sizeof(cfrp_channel_t));

  for (int i = 0; i < last; i++) {
    ch = frp->channels + i;
    log_debug("sync channel send %d -> %d", channel->fd, ch->fd);
    cfrp_channel_send(ch, &msg);
  }

  return C_SUCCESS;
}

extern int cfrp_channel_event_add(struct cfrp *frp, struct cfrp_channel *ch) {
  cfrp_list_t *head = &frp->channel_event.list;
  cfrp_event_t *ev  = cfrp_malloc(sizeof(cfrp_event_t));

  __non_null__(ev, C_ERROR);

  ev->entry.channel = ch;
  ev->type          = CFRP_CHANNEL;

  list_add(&ev->list, head);

  return cfrp_epoll_add(frp->epoll_private, ev);
}

extern int cfrp_channel_event_del(struct cfrp *frp, struct cfrp_channel *ch) {
  cfrp_list_t *head = &frp->channel_event.list;
  cfrp_event_t *entry;

  list_foreach_entry(entry, head, list) {
    if (entry->entry.channel == ch) {
      list_del(entry->list.prev, entry->list.next);
      cfrp_epoll_del(frp->epoll_private, entry);
      cfrp_channel_close(ch->fd);
      cfrp_free(entry);
      return C_SUCCESS;
    }
  }

  return C_ERROR;
}

int cfrp_channel_event_handler(struct cfrp *frp, struct cfrp_channel *ch) {
  cfrp_server_t *server;
  cfrp_channel_t *channel;
  cfrp_sock_t *sock;
  cfrp_cmsg_t msg;

  if (cfrp_channel_recv(ch, &msg) <= 0) {
    log_debug("open channel failure. msg: %s", CFRP_SYS_ERROR);
    return C_ERROR;
  }

  switch (msg.cmd) {
    case CFRP_CMD_OPEN_CHANNEL: {
      channel      = &msg.data.channel;
      channel->fd  = msg.fd;
      channel->cmd = CFRP_CMD_NUI;
      cfrp_memcpy(&frp->channels[channel->slot], channel, sizeof(cfrp_channel_t));
      log_debug("proc channel cfrp.channels[%d] -> %d", channel->slot, channel->fd);
    } break;

    case CFRP_CHANNEL_MASOCK:
    case CFRP_CANNEL_MPSOCK:
    case CFRP_CHANNEL_CPSOCK:
    case CFRP_CHANNEL_CLSOCK: {
      server   = (cfrp_server_t *)frp->entry;
      sock     = &msg.data.sock;
      sock->fd = msg.fd;
      if (msg.cmd == CFRP_CHANNEL_CLSOCK) {

      } else {
        cfrp_memcpy(cfrp_pair_last(server->sock_pair), sock, sizeof(cfrp_sock_t));
        log_info("proc channel sock pair %s:%d#%d", sock->host, sock->port, sock->fd);
      }
    } break;

    default: {
      log_warning("proc channel illegal");
      return C_ERROR;
    };
  }
  return C_SUCCESS;
}
