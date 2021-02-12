#include "net.h"

// 等待数
#ifndef WAIT_NUM
#define WAIT_NUM 10
#endif

// 监听数
#ifndef LISTEN_NUM
#define LISTEN_NUM 10
#endif

struct cfrp_event
{
    void *handle;
};

extern struct cfrp_event *make_event();

extern int event_register(struct cfrp_event *ev, struct sock *sk);

extern int event_remove(struct cfrp_event *ev, struct sock *sk);

extern int event_close(struct cfrp_event *ev);

extern int event_wait(struct cfrp_event *ev);
