#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "cfrp.h"
#include "net.h"
#include "lib.h"
#include "logger.h"
#include "job.h"

static int cfrp_server_start(struct cfrp *frp)
{

   
    job_start(frp->job, cfrp_cpu);
}

static int cfrp_server_kill(struct cfrp *frp, char *sid)
{
    struct cfrp_session *sn = cfrp_sdel(frp, sid);
    if (!sn)
        return C_ERROR;
    worker_t *wk = (worker_t *)sn->worker;
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
    if (!(frp->job = malloc(sizeof(struct cfrp_job))))
    {
        sock_close(frp->msk);
        free(frps);
        log_error("cfrp job malloc failure!");
        return NULL;
    }
    if (!(frp->sessions->head = malloc(sizeof(struct list_head))))
    {
        sock_close(frp->msk);
        free(frp->job);
        free(frps);
        log_error("cfrp session malloc failure!");
        return NULL;
    }
    INIT_LIST_HEAD(frp->sessions->head);
    mappings &&memcpy(frp->mappings, mappings, sizeof(struct cfrp_mapping *));
    if (!(frps->op = malloc(sizeof(struct cfrp_operating))))
    {
        sock_close(frp->msk);
        free(frp->job);
        free(frp->sessions->head);
        free(frps);
        log_error("cfrp operating error");
        return NULL;
    }
    frps->pid = getpid();
    frps->frp = frp;
    frps->op = &server_operating;
    return frps;
}