#include <sys/epoll.h>

#include "lib.h"
#include "logger.h"
#include "nevent.h"

struct cfrp_epoll *cfrp_epoll_create() {
  struct cfrp_epoll *ep = (struct cfrp_epoll *)cfrp_malloc(sizeof(struct cfrp_epoll));
  if (!ep || (ep->efd = epoll_create(20)) < 0) {
    return C_ERROR;
  }
  INIT_LIST_HEAD(&ep->events.list);
  return ep;
}

extern int cfrp_epoll_wait(struct cfrp_epoll *epoll, struct sock_event *sk_events, int max_event, int timeout) {
  log_debug("wait for a sock event");
  struct epoll_event events[max_event - 1], *ev;
  struct sock_event *sev;
  int num = 0;
  INIT_LIST_HEAD(&sk_events->list);
  if ((num = epoll_wait(epoll->efd, events, max_event, timeout)) < 0) {
    log_error("epoll wait error: %s", SYS_ERROR);
    return C_ERROR;
  }
  for (int i = 0; i < num; i++) {
    ev = events + i;
    sev = sk_events + i + 1;
    sev->sk = (fsock_t *)ev->data.ptr;
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
  struct sock_event *sk_event = cfrp_malloc(sizeof(struct sock_event));
  __non_null__(sk_event, C_ERROR);
  cfrp_memcpy(sk_event, event, sizeof(struct sock_event));
  cfrp_zero(&sk_event->list, sizeof(struct list_head));
  sock_t *sk = sk_event->sk;
  log_debug("listen for a sock event. %s:%d#%d", sk->host, sk->port, sk->fd);
  struct epoll_event ev;
  ev.events = EPOLLET | EPOLLIN | EPOLLERR, ev.data.fd = sk->fd;
  ev.data.ptr = sk;
  list_add(&sk_event->list, &epoll->events.list);
  return epoll_ctl(epoll->efd, EPOLL_CTL_ADD, sk->fd, &ev);
}

int cfrp_epoll_reuse(struct cfrp_epoll *epoll, struct sock_event *event) {
  return C_ERROR;
}

int cfrp_epoll_del(struct cfrp_epoll *epoll, struct sock_event *event) {
  sock_t *sk = event->sk;
  return epoll_ctl(epoll->efd, EPOLL_CTL_DEL, sk->fd, NULL);
}

int cfrp_epoll_close(struct cfrp_epoll *epoll) {
  __non_null__(epoll, C_ERROR);
  return close(epoll->efd);
}

int cfrp_epoll_clear(struct cfrp_epoll *epoll) {
  __non_null__(epoll, C_ERROR);
  log_debug("clear event set");
  struct list_head *entry;
  struct sock_event *ev;
  list_foreach(entry, &epoll->events.list) {
    ev = list_entry(entry, struct sock_event, list);
    if (epoll_ctl(epoll->efd, EPOLL_CTL_DEL, ev->sk->fd, NULL) < 0) {
      log_error("del sock event error, %s:%d, msg:%s", ev->sk->host, ev->sk->port, SYS_ERROR);
    }
    list_del(ev->list.prev, ev->list.next);
    cfrp_free(ev);
  }
  return C_SUCCESS;
}