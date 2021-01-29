/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2021-01-29 11:45:01
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cfrpx/include/cfrp.h
 */
#ifndef __CFRP_H__
#define __CFRP_H__

#include <stdio.h>
#include <unistd.h>

#include "session.h"
#include "logger.h"
#include "net.h"
#include "error.h"
#include "lib.h"

struct cfrp
{
    int pid;
    void *ptr;
};

extern struct cfrp *make_cfrpc(const uint client_port);

extern struct cfrp *make_cfrps(const uint bind_port);

extern struct session *get_session(struct cfrp *frp);

extern int start_cfrp(struct cfrp *frp);

extern int stop_cfrp(struct cfrp *frp);

#endif