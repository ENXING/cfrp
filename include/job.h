#ifndef __JOB_H__
#define __JOB_H__
#include <unistd.h>
#include <stdlib.h>
#include "cfrp.h"

static struct cfrp_worker *make_worker(pid_t pid)
{
    struct cfrp_worker *wk;
}


static int job_start(struct cfrp_job *job, uint num)
{
    pid_t mpid = getpid();
    pid_t pid;
    struct list_head *head = (struct list_head *)malloc(sizeof(struct list_head));
    for (int i = 0; i < num; i++)
    {
        if (getpid() != mpid)
            break;
        pid = fork();
        if (pid > 0)
        {
            // make_worker(pid, );
        }
        else if (pid < 0)
        {
        }
    }
}

#endif