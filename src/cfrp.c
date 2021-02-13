#include <stdlib.h>
#include <sys/epoll.h>
#include "cfrp.h"

/**
 * 启动
*/
int cfrp_start(struct cfrp_context *ctx)
{
    return ctx->op->start(ctx->frp);
}

/**
 * 重新加载
*/
int cfrp_reload(struct cfrp_context *ctx)
{
    return ctx->op->reload(ctx->frp);
}
/**
 * 杀死一条任务
*/
int cfrp_kill(struct cfrp_context *ctx, char *sid)
{
    return ctx->op->kill(ctx->frp, sid);
}

/**
 * 停止
*/
int cfrp_stop(struct cfrp_context *ctx)
{
    return ctx->op->stop(ctx->frp);
}
