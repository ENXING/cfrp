#include "cfrp.h"
#include "net.h"
#include "lib.h"
#include "logger.h"
#include "job.h"
#include "mutex.h"
#include "nevent.h"
#include "session.h"

static void cfrp_server_accept(struct cfrp *frp, fworker_t *wk)
{
    log_debug("accept sock");
    fserver_t *server = (fserver_t *)frp->entry;
    //  将所有fd加入事件集
    struct list_head *entry;
    struct sock_event *event;
    list_foreach(entry, &server->sock_accept.list)
    {
        event = list_entry(entry, struct sock_event, list);
        cfrp_epoll_add(frp->epoll, event->sk);
    }
    struct sock_event events[10], *ev;
    int num = 0;
    if ((num = cfrp_epoll_wait(frp->epoll, events, 10, 5000)) < 0)
    {
        log_error("epoll error");
    };
    sock_t *client_sk;
    list_foreach(entry, &events->list)
    {
        ev = list_entry(entry, struct sock_event, list);
        if (ev->events & CFRP_EVENT_IN)
        {
            client_sk = sock_accept(ev->sk);
            // 
        }
    };
}

static void cfrp_handler_read_or_write(fworker_t *wk)
{
    // log_debug("read or wirte");
    sleep(1);
}

static void cfrp_server_process_handler(fworker_t *wk)
{
    cfrp_t *frp = (cfrp_t *)wk->ctx;
    LOOP
    {
        cfrp_mutex(frp, wk, cfrp_server_accept);
        cfrp_handler_read_or_write(wk);
    }
}

static void init_cfrp_server(struct cfrp *frp)
{
    frp->job.lock->mutex = CFRP_MUTEX_UNLOCK;
    set_noblocking(frp->msk->fd);
    fserver_t *server = (fserver_t *)frp->entry;
    INIT_LIST_HEAD(&server->sock_accept.list);
    struct sock_event *ev = (struct sock_event *)cfrp_malloc(sizeof(struct sock_event));
    ev->sk = frp->msk;
    list_add(&ev->list, &server->sock_accept.list);
}

static int cfrp_server_start(struct cfrp *frp)
{
    log_debug("cfrp init process");
    init_cfrp_server(frp);
    cfrp_start_worker_process(frp, cfrp_server_process_handler);
    log_debug("cfrp process started!");
    struct cfrp_accept_sock *ask;
    struct list_head *entry;
    LOOP
    {
        sleep(1);
    }
}

static int cfrp_server_kill(struct cfrp_session *slist, char *sid)
{
    struct cfrp_session *sn = cfrp_sdel(slist, sid);
    __non_null__(sn, C_ERROR);
    return C_SUCCESS;
}

static int cfrp_server_stop(struct cfrp *frp)
{
}

static int cfrp_server_restart(struct cfrp *frp)
{
}

static int cfrp_server_reload(struct cfrp *frp)
{
}

static struct cfrp_operating server_operating = {
    .start = cfrp_server_start,
    .restart = cfrp_server_restart,
    .stop = cfrp_server_stop,
    .kill = cfrp_server_kill};

cfrps *make_cfrps(char *bind_addr, uint port, struct cfrp_mapping *mappings, int argc, char **argv)
{
    int err = 0;
    cfrps *frps = cfrp_malloc(sizeof(cfrps));
    cfrp_t *frp = cfrp_malloc(sizeof(cfrp_t));
    struct sock *msk = make_tcp(port, bind_addr);
    shmtable_t *shm = cfrp_shmget(SHM_SIZE);
    struct cfrp_server *server = cfrp_malloc(sizeof(struct cfrp_server));
    struct cfrp_epoll *epoll = cfrp_epoll_create();
    __null__(frps)
    {
        log_error("cfrps malloc failure!");
        err = 1;
    }
    __null__(frp)
    {
        log_error("cfrps malloc frp failure!");
        err = 1;
    }
    __null__(msk)
    {
        log_error("cfrps tcp create failure! %s", SYS_ERROR);
        err = 1;
    }
    __null__(shm)
    {
        log_error("cfrps share memory create failure! %s", SYS_ERROR);
        err = 1;
    }
    __null__(server)
    {
        log_error("cfrps server malloc failure!");
        err = 1;
    }
    __null__(epoll)
    {
        log_error("cfrps epoll create failure!");
        err = 1;
    }
    if (err)
    {
        cfrp_free(frp);
        cfrp_free(frps);
        sock_close(msk);
        cfrp_shmfree(shm);
        cfrp_epoll_close(epoll);
        return NULL;
    }
    // base cfrp
    frp->msk = msk;
    frp->job.lock = cfrp_shmblock((shmtable_t *)shm, sizeof(struct cfrp_lock));
    frp->ctx = frps;
    frp->entry = server;
    frp->epoll = epoll;
    // cfrp context
    frps->frp = frp;
    frps->pid = getpid();
    frps->op = &server_operating;
    return frps;
}