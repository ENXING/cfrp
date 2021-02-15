#ifndef __EVENT_H__
#define __EVENT_H__
#include "cfrp.h"
extern int cfrp_epoll_create();
extern int cfrp_epoll_wait(struct cfrp_epoll *epoll);
extern int cfrp_epoll_add(struct cfrp_epoll *epoll, sock_t *sk);
extern int cfrp_epoll_reuse(struct cfrp_epoll *epoll, sock_t *sk);
extern int cfrp_epoll_del(struct cfrp_epoll *epoll, sock_t *sk);
#endif