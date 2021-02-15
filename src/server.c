#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "cfrp.h"
#include "net.h"
#include "lib.h"
#include "logger.h"
#include "job.h"
#include "mutex.h"
#include "event.h"

static void cfrp_event_accept(struct cfrp *frp, worker_t *wk)
{
    log_debug("accept sock");
    sleep(10);
}

static void cfrp_process_forward(worker_t *wk)
{
}

static void cfrp_server_process_handler(worker_t *wk)
{
    cfrp_t *frp = (cfrp_t *)wk->ctx;
    LOOP
    {
        cfrp_mutex(frp, wk, cfrp_event_accept);
        cfrp_process_forward(wk);
    }
}

static void init_cfrp_server(struct cfrp *frp)
{
    frp->job.lock->mutex = CFRP_MUTEX_UNLOCK;
}

static int cfrp_server_start(struct cfrp *frp)
{
    log_debug("cfrp init process");
    init_cfrp_server(frp);
    cfrp_start_worker_process(frp, cfrp_server_process_handler);
    log_debug("cfrp process started!");
    LOOP
    {
        sleep(1);
    }
}

static int cfrp_server_kill(struct cfrp *frp, char *sid)
{
    struct cfrp_session *sn = cfrp_sdel(frp, sid);
    if (!sn)
        return C_ERROR;
    worker_t *wk = (worker_t *)sn->wk;
    wk->op->kill(wk, sn);
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

cfrps *make_cfrps(char *bind_addr, uint port, struct cfrp_mapping *mappings)
{
    cfrps *frps = malloc(sizeof(cfrps));
    if (!frps)
    {
        log_error("server malloc failure!");
        return NULL;
    }
    struct cfrp *frp = malloc(sizeof(struct cfrp));
    if (!(frp->msk = make_tcp(port, bind_addr)))
    {
        log_error("tcp create failure!, socket: %s:%d, msg: %s", bind_addr, port, SYS_ERROR);
        return NULL;
    }
    if (!(frp->sessions.head = malloc(sizeof(struct list_head))))
    {
        sock_close(frp->msk);
        free(frps);
        log_error("cfrp session malloc failure!");
        return NULL;
    }
    INIT_LIST_HEAD(frp->sessions.head);
    mappings &&memcpy(&frp->mappings, mappings, sizeof(struct cfrp_mapping *));

    // 共享内存
    if (!(frp->shm = cfrp_shmget(SHM_SIZE)))
    {
        sock_close(frp->msk);
        free(frp->sessions.head);
        free(frps);
        return NULL;
    }
    frp->job.lock = cfrp_shmblock((shmtable_t *)frp->shm, sizeof(struct cfrp_lock));
    frps->pid = getpid();
    frps->frp = frp;
    frps->op = &server_operating;
    return frps;
}