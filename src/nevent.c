#include <sys/epoll.h>

#include "nevent.h"
#include "lib.h"
#include "logger.h"

struct cfrp_epoll *cfrp_epoll_create()
{
    struct cfrp_epoll *ep = (struct cfrp_epoll *)cfrp_malloc(sizeof(struct cfrp_epoll));
    if (!ep || (ep->efd = epoll_create(20)) < 0)
    {
        return C_ERROR;
    };
    return ep;
}

extern int cfrp_epoll_wait(struct cfrp_epoll *epoll, struct sock_event *sk_events, int max_event, int timeout)
{
    log_debug("waiting sock...");
    struct epoll_event events[max_event - 1], *ev;
    struct sock_event *sev;
    int num = 0;
    INIT_LIST_HEAD(&sk_events->list);
    if ((num = epoll_wait(epoll->efd, events, max_event, timeout)) < 0)
    {
        log_error("epoll wait error: %s", SYS_ERROR);
        return C_ERROR;
    }
    for (int i = 0; i < num; i++)
    {
        ev = events + i;
        sev = sk_events + i + 1;
        sev->sk = (fsock_t *)ev->data.ptr;
        if (ev->events & (EPOLLIN | EPOLLOUT))
        {
            sev->events = CFRP_EVENT_IN | CFRP_EVENT_OUT;
        }
        else if (ev->events & EPOLLIN)
        {
            sev->events = CFRP_EVENT_IN;
        }
        else if (ev->events & EPOLLOUT)
        {
            sev->events = CFRP_EVENT_OUT;
        }
        else if (ev->events & EPOLLERR)
        {
            sev->events = EPOLLERR;
        }
        list_add(&sev->list, &sk_events->list);
    }
    log_info("epoll event num: %d", num);
    return num;
}

int cfrp_epoll_add(struct cfrp_epoll *epoll, fsock_t *sk)
{
    log_debug("epoll add: %s:%d#%d", sk->host, sk->port, sk->fd);
    struct epoll_event ev = {
        .data = {
            .fd = sk->fd,
            .ptr = sk},
        .events = EPOLLET | EPOLLIN};
    return epoll_ctl(epoll->efd, EPOLL_CTL_ADD, sk->fd, &ev);
}

int cfrp_epoll_reuse(struct cfrp_epoll *epoll, fsock_t *sk)
{
}

int cfrp_epoll_del(struct cfrp_epoll *epoll, fsock_t *sk)
{
    return epoll_ctl(epoll->efd, EPOLL_CTL_DEL, sk->fd, NULL);
}

int cfrp_epoll_close(struct cfrp_epoll *epoll)
{
    __non_null__(epoll, C_ERROR);
    return close(epoll->efd);
}