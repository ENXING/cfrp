#include <sys/epoll.h>
#include <stdlib.h>
#include "cevent.h"
#include "logger.h"

#define EVENT_EPOLL(ev) ((struct event_epoll * ep) ev)

struct event_epoll
{
    int efd;
};

struct cfrp_event *make_event()
{
    int efd = -1;
    if ((efd = epoll_create(LISTEN_NUM)) < 0)
    {
        log_error("make event error!");
        return NULL;
    }
    struct cfrp_event *ev = calloc(sizeof(struct cfrp_event), 1);
    struct event_epoll *ep = calloc(sizeof(struct event_epoll), 1);
    ep->efd = efd;
    ev->handle = ep;
    return ev;
}

int event_register(struct cfrp_event *ev, struct sock *sk)
{
}

int event_remove(struct cfrp_event *ev, struct sock *sk)
{
}

int event_close(struct cfrp_event *ev)
{
}

int event_wait(struct cfrp_event *ev)
{
}