#include <stdlib.h>
#include <sys/epoll.h>
#include "cfrp.h"
#include "list.h"

extern cfrps *make_cfrps()
{
}

extern cfrpc *make_cfrpc()
{
}

/**
 * 启动
*/
int cfrp_start(struct __cfrp *frp)
{
}
/**
 * 重新加载
*/
int cfrp_reload(struct __cfrp *frp)
{
}
/**
 * 杀死一条任务
*/
int cfrp_kill(struct __cfrp *frp, int cid)
{
}
/**
 * 停止
*/
int cfrp_stop(struct __cfrp *frp)
{
}

/**
 * 数据转发
*/
extern int cfrp_forward(struct sock *dest, struct sock *src)
{

    sock_recv(src, NULL, 0);
}
