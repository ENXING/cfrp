#ifndef __CFRP_H__
#define __CFRP_H__
#include "types.h"
#include "net.h"

// 会话唯一标识长度
#define SID_LEN 18
// 正常传输
#define CFRP_PACKET_NORMAL 0x01
// 分包传输
#define CFRP_PACKET_SUB 0x02
// 协议头大小
#define CFRP_HEAD_SIZE sizeof(struct cfrp_protocol)

/**
 * cfrp传输协议
 * 服务端与客户端必须要按照这个协议通讯
*/
struct cfrp_protocol
{
    // 本次会话类型
    // 0x01: 正常传输
    // 0x02: 断开连接
    // 0x03: 会话终止
    char type;
    // 本次会话id
    char sid[SID_LEN];
    // 转发到的端口
    uint mapping_port;
    // 校验类型
    char verify_type;
    // 校验长度
    uint8 verify_len;
    // 数据包信息, 实际大小信息
    struct
    {
        // 数据包类型
        // 0x01: 正常传输, 0x01: 分包传输
        char type;
        // 总大小
        size_t total;
        // 实际大小
        size_t full;
    } packet_info;
};

/**
 * cfrp payload
*/
struct cfrp_payload
{
    // 协议头
    struct cfrp_protocol head;
    // 传输的数据
    char *data;
};

struct cfrp_list_head
{
    struct cfrp_list_head *next;
    struct cfrp_list_head *prev;
};

/**
 * 映射信息
*/
struct cfrp_mapping
{
    char *addr;
    uint port;
    struct cfrp_list_head *head;
};

/**
 * 会话信息
*/
struct cfrp_session
{
    char sid[SID_LEN];
    struct sock *sk;
    void *ptr;
    struct cfrp_list_head *head;
};

struct cfrp_worker
{
    int pid;
    void *ctx;
    struct cfrp_session *session;
};

struct cfrp_job
{
    int lock;
    void *wating_worker;
    struct cfrp_worker *workers;
    int (*nofity)(struct cfrp_worker *woker);
};

struct cfrp
{
    // 主要监听服务
    struct sock *msk;
    // 映射信息
    struct cfrp_mapping *mappings;
    // 会话信息
    struct cfrp_session *sessions;
    // 工作
    struct cfrp_job *job;
};

struct cfrp_operating
{
    int (*start)(struct cfrp *);
    int (*stop)(struct cfrp *);
    int (*reload)(struct cfrp *);
    int (*restart)(struct cfrp *);
    int (*kill)(struct cfrp *, char *sid);
};

struct cfrp_context
{
    int pid;
    struct cfrp *frp;
    struct cfrp_operating *op;
};

typedef struct cfrp_context cfrps;
typedef struct cfrp_context cfrpc;

extern cfrps *make_cfrps(char *bind_addr, uint port, struct cfrp_mapping *mappings);

extern cfrpc *make_cfrpc(char *client_addr, uint port, struct cfrp_mapping *mappings);

/**
 * 启动
*/
extern int cfrp_start(struct cfrp_context *ctx);

/**
 * 重新启动
*/
extern int cfrp_restart(struct cfrp_context *ctx);
/**
 * 重新加载
*/
extern int cfrp_reload(struct cfrp_context *ctx);
/**
 * 杀死一个会话
*/
extern int cfrp_kill(struct cfrp_context *ctx, char *sid);
/**
 * 停止
*/
extern int cfrp_stop(struct cfrp_context *ctx);

extern int cfrp_verify(struct cfrp *frp, char *sid);

extern int cfrp_proto(struct cfrp *frp, struct cfrp_protocol *dest, char *sid);

extern int cfrp_recv(struct cfrp *frp, char *sid, void *buff, size_t size);

extern int cfrp_send(struct cfrp *frp, char *sid, void *data, size_t size);

extern int cfrp_tranform(struct cfrp *frp, char *dest_sid, char *src_sid, size_t size);

extern struct cfrp_session *cfrp_sadd(struct cfrp *frp, struct cfrp_session *session);

extern struct cfrp_session *cfrp_sget(struct cfrp *frp, char *sid);
/**
 * 生成一个唯一会话Id
*/
extern char *cfrp_gensid();

#endif