#include "cfrp.h"
#include "config.h"

/**
 * 开启通道
 */
#define CFRP_CHANNEL_OPEN 1

/**
 * 退出程序
 */
#define CFRP_CHANNEL_EXIT 2

/**
 * 服务端与客户端通讯的sock
 */
#define CFRP_CHANNEL_MSOCK 3

/**
 * 映射端sock
 */
#define CFRP_CANNEL_MSOCK 4

/**
 * 客户端映射连接的sock
 */
#define CFRP_CHANNEL_CSOCK 5

/**
 * 关闭sock
 */
#define CFRP_CHANNEL_SOCK_CLOSE 6

extern int cfrp_channel_send(struct cfrp_channel *ch, struct cfrp_cmsg *msg);

extern int cfrp_channel_recv(struct cfrp_channel *ch, struct cfrp_cmsg *msg);