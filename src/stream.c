#include <stdlib.h>
#include "stream.h"
#include "logger.h"

typedef int (*__sock_stream_io)(void *sk, void *buff, size_t size);
typedef int (*__sock_stream_op)(void *sk);

static int *sock_brecv(struct sock *sk, void *buff, size_t size)
{
    log_info("brecv");
}

static int *sock_bsend(struct sock *sk, void *data, size_t size)
{
    log_info("bsend");
}

static int *sock_bflush(struct sock *sk)
{
}

static int *sock_bclose(struct sock *sk)
{
}

/**
 * 正常发送
*/
struct stream_operating *stream_base()
{
    static struct stream_operating op = {
        .send = (__sock_stream_io)sock_brecv,
        .recv = (__sock_stream_io)sock_bsend,
        .flush = (__sock_stream_op)sock_bflush,
        .close = (__sock_stream_op)sock_bclose};
    return &op;
}

/**
 * 带缓存发送
*/
struct stream_operating *stream_buffer(struct buffer *buf)
{
}

/**
 * 分包发送
*/
struct stream_operating *stream_subpackage(size_t total, size_t package)
{
}