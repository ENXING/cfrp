#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "cfrp.h"
#include "net.h"
#include "lib.h"
#include "logger.h"

static int cfrp_server_start(struct cfrp *frp)
{
    int cpu_num = cfrp_cpu;
    pid_t mpid = getpid();
    pid_t pid;
    log_info("init");
    for (int i = 0; i < cpu_num; i++)
    {
        if (getpid() != mpid)
            break;
        pid = fork();
        if (pid < 0)
        {
            perror("fork error");
        }
        else if (pid > 0)
        {
            log_info("pid: %d, ppid: %d, mpid: %d", pid, getppid(), mpid);
        }
    }
}

static int cfrp_server_kill(struct cfrp *frp, char *sid)
{
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
        log_error("cfrp job malloc failure!");
        return NULL;
    }
    mappings &&memcpy(frp->mappings, mappings, sizeof(struct cfrp_mapping *));
    if (!(frps->op = malloc(sizeof(struct cfrp_operating))))
    {
        log_error("cfrp operating error");
    }
    frps->pid = getpid();
    frps->frp = frp;
    frps->op = &server_operating;
    return frps;
}