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
#define CFRP_CHANNEL_MASOCK 3

/**
 * 映射端sock
 */
#define CFRP_CANNEL_MPSOCK 4

/**
 * 客户端映射连接的sock
 */
#define CFRP_CHANNEL_CPSOCK 5

/**
 * 关闭sock
 */
#define CFRP_CHANNEL_CLSOCK 6

/**
 * 关闭 session
 */
#define CFRP_CHANNEL_CLSN 7

/**
 * 打开 session
 */
#define CFRP_CHANNEL_OPSN 8

/**
 * 同步客户端 sock
 */
#define CFRP_CHANNEL_CSOCK 10

extern int cfrp_channel_send(struct cfrp_channel *ch, struct cfrp_cmsg *msg);

extern int cfrp_channel_recv(struct cfrp_channel *ch, struct cfrp_cmsg *msg);