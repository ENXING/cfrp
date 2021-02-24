#ifndef __EVENT_H__
#define __EVENT_H__
#include "cfrp.h"

#define CFRP_CHANNEL 10
#define CFRP_SOCK 20

#define CFRP_EVENT_IN 1
#define CFRP_EVENT_OUT 2
#define CFRO_EVENT_ERR 3

extern struct cfrp_epoll *cfrp_epoll_create();
extern int cfrp_epoll_wait(struct cfrp_epoll *epoll, struct sock_event *events, int max_event, int timeout);
extern int cfrp_epoll_add(struct cfrp_epoll *epoll, struct sock_event *event);
extern int cfrp_epoll_reuse(struct cfrp_epoll *epoll, struct sock_event *event);
extern int cfrp_epoll_del(struct cfrp_epoll *epoll, struct sock_event *event);
extern int cfrp_epoll_close(struct cfrp_epoll *epoll);
extern int cfrp_epoll_clear(struct cfrp_epoll *epoll);
#endif