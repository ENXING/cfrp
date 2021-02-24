#include <sys/epoll.h>

#include "lib.h"
#include "logger.h"
#include "nevent.h"

static inline int cfrp_epoll_get_fd(struct sock_event *event) {
  cfrp_sock_t *sk;
  cfrp_channel_t *ch;
  int fd;
  if (event->type == CFRP_SOCK) {
    sk = event->entry.sk;
    fd = ch->fd;
  } else if (event->type == CFRP_CHANNEL) {
    ch = event->entry.channel;
    fd = ch->fd;
  } else {
    return C_ERROR;
  }
  return fd;
};

struct cfrp_epoll *cfrp_epoll_create() {

  struct cfrp_epoll *ep = (struct cfrp_epoll *)cfrp_malloc(sizeof(struct cfrp_epoll));

  if (!ep || (ep->efd = epoll_create(20)) < 0) {
    return C_ERROR;
  }

  INIT_LIST_HEAD(&ep->events.list);

  log_debug("cfrp create epoll -- %d", ep->efd);
  ep->pid = getpid();

  return ep;
}

extern int cfrp_epoll_wait(struct cfrp_epoll *epoll, struct sock_event *sk_events, int max_event, int timeout) {
  log_debug("wait for a sock event");

  struct epoll_event events[max_event + 1], *ev;
  struct sock_event *sev;
  int num = 0;

  INIT_LIST_HEAD(&sk_events->list);

  if ((num = epoll_wait(epoll->efd, events, max_event, timeout)) < 0) {
    log_error("epoll wait error: %s", SYS_ERROR);
    return C_ERROR;
  }

  for (int i = 0; i < num; i++) {
    ev  = events + i;
    sev = sk_events + i + 1;

    cfrp_memcpy(sev, ev->data.ptr, sizeof(cfrp_event_t));

    if (ev->events & (EPOLLIN | EPOLLOUT)) {
      sev->events = CFRP_EVENT_IN | CFRP_EVENT_OUT;
    } else if (ev->events & EPOLLIN) {
      sev->events = CFRP_EVENT_IN;
    } else if (ev->events & EPOLLOUT) {
      sev->events = CFRP_EVENT_OUT;
    } else if (ev->events & EPOLLERR) {
      sev->events = CFRO_EVENT_ERR;
    }

    list_add(&sev->list, &sk_events->list);
  }

  if (num > 0)
    log_debug("waiting sock number of events: %d", num);
  return num;
}

int cfrp_epoll_add(struct cfrp_epoll *epoll, struct sock_event *event) {
  __non_null__(event, C_ERROR);

  cfrp_sock_t *sk         = NULL;
  cfrp_channel_t *channel = NULL;
  int fd                  = cfrp_epoll_get_fd(event);

  struct sock_event *sk_event = cfrp_malloc(sizeof(struct sock_event));
  struct epoll_event ev;

  __non_null__(sk_event, C_ERROR);

  cfrp_memcpy(sk_event, event, sizeof(struct sock_event));
  cfrp_memzero(&sk_event->list, sizeof(struct list_head));

  if (event->type == CFRP_SOCK) {
    sk = event->entry.sk;
    log_debug("listen for a sock event. %s:%d#%d", sk->host, sk->port, sk->fd);
  } else if (event->type == CFRP_CHANNEL) {
    channel = event->entry.channel;
    log_debug("listen for a channel event. -- %d", channel->fd)
  } else {
    cfrp_free(sk_event);
    return C_ERROR;
  }

  sk_event->ptr = event;

  ev.events   = EPOLLET | EPOLLIN | EPOLLERR;
  ev.data.fd  = fd;
  ev.data.ptr = sk_event;

  list_add(&sk_event->list, &epoll->events.list);

  return epoll_ctl(epoll->efd, EPOLL_CTL_ADD, fd, &ev);
}

int cfrp_epoll_reuse(struct cfrp_epoll *epoll, struct sock_event *event) {
  return C_ERROR;
}

int cfrp_epoll_del(struct cfrp_epoll *epoll, struct sock_event *event) {
  int fd = cfrp_epoll_get_fd(event);
  if (fd <= 0)
    return C_ERROR;
  return epoll_ctl(epoll->efd, EPOLL_CTL_DEL, fd, NULL);
}

int cfrp_epoll_close(struct cfrp_epoll *epoll) {
  __non_null__(epoll, C_ERROR);
  if (epoll->pid == cfrp_getpid()) {
    cfrp_epoll_clear(epoll);
  }
  return close(epoll->efd);
}

int cfrp_epoll_clear(struct cfrp_epoll *epoll) {
  __non_null__(epoll, C_ERROR);
  log_debug("clear event set");
  struct list_head *entry;
  struct sock_event *ev;
  int fd;
  list_foreach(entry, &epoll->events.list) {
    ev = list_entry(entry, struct sock_event, list);
    fd = cfrp_epoll_get_fd(ev);
    if (epoll_ctl(epoll->efd, EPOLL_CTL_DEL, fd, NULL) < 0) {
      log_error("del sock event error ---[%d], msg: %s", fd, SYS_ERROR);
    }
    list_del(ev->list.prev, ev->list.next);
    cfrp_free(ev);
  }
  return C_SUCCESS;
}