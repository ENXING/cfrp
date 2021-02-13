#ifndef __CHANNEL_H__
#define __CHANNEL_H__
#include "net.h"
#include "buffer.h"

/**
 * 正常发送
*/
extern struct stream_operating *stream_base();

/**
 * 带缓存发送
*/
extern struct stream_operating *stream_buffer(struct buffer *buf);

/**
 * 分包发送
*/
extern struct stream_operating *stream_subpackage(size_t total, size_t package);

#endif
