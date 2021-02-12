#ifndef __CFRP_H__
#define __CFRP_H__

#include <stdio.h>
#include <unistd.h>

#include "logger.h"
#include "net.h"
#include "error.h"
#include "types.h"
#include "list.h"

#define CFRP_SERVER 0x01
#define CFRP_CLIENT 0x02

#define SESSION_LEN 18

#define CFRP_PROCESS_NUM 4
#define CFRP_CONNECT_NUM 100
#define CFRP_MAX_PRCOESS 20

/** payload mask */
typedef enum
{
    CFRP_MASK_TYPE,
    CFRP_MASK_KEY_TYPE,
    CFRP_MASK_KEY_LEN,
    CFRP_MASK_DATA_LEN
} cfrp_mask;
/** end */

struct session
{
    char *sid;
    void *ptr;
};

struct session_map
{
    int map_size;
};

struct mapping
{
    struct sock *sk;
};

struct mapping_map
{
    int mid;
};

struct worker
{
    int pid;
    int (*notify)(struct __cfrp frp, struct sock *sk);
    struct session_map sp;
};

struct worker_pool
{
    int lock;
    uint woker_num;
};

struct task_queueint
{
    int lock;
    void *ptr;
    int (*func)(struct worker);
};

struct __cfrp
{
    // 进程号
    int pid;
    // 服务端信息
    struct sock sk;
    // 映射信息
    struct mapping_map mp;
    // 工作池
    struct worker_pool wp;
    // task
    struct task_queue tq;
};

typedef struct __cfrp cfrps;
typedef struct __cfrp cfrpc;

struct cfrp_payload
{
    struct
    {
        // 会话类型
        uint8_t type;
        char token[SESSION_LEN];
        uint mapping_port;
        char verify_type;
        uint8_t verify_len;
        char verify;
        short data_len;
    } head;
    char *data;
};

extern cfrps *make_cfrps();

extern cfrpc *make_cfrpc();

/**
 * 启动
*/
extern int cfrp_start(struct __cfrp *frp);
/**
 * 重新加载
*/
extern int cfrp_reload(struct __cfrp *frp);
/**
 * 杀死
*/
extern int cfrp_kill(struct __cfrp *frp, int cid);
/**
 * 停止
*/
extern int cfrp_stop(struct __cfrp *frp);

/**
 * 转发
*/
extern int cfrp_forward(struct sock *dest, struct sock *src);

#endif