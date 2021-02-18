#ifndef __CHANNEL_H__
#define __CHANNEL_H__
#include "net.h"
#include "buffer.h"

/**
 * 正常发送
*/
extern struct stream_operating *stream_base(sock_t *sk);

/**
 * 带缓存发送
*/
extern struct stream_operating *stream_buffer(sock_t *sk, struct buffer *buf);

/**
 * 分包发送
*/
extern struct stream_operating *stream_subpackage(sock_t *sk, size_t total, size_t package);

#endif
