#ifndef __CFRP_H__
#define __CFRP_H__
#include "list.h"
#include "net.h"
#include "types.h"

// 会话唯一标识长度
#define SID_LEN 18
// 协议头大小
#define CFRP_HEAD_SIZE sizeof(struct cfrp_protocol)
// 最大等待客户端连接数量
#define CFRP_MAX_WAIT_CONN_NUM 10
#define cfrp_mpair(sk) (sk)     // 主要
#define cfrp_spair(sk) (sk + 1) // 次要

#define CFRP_SHMSIZE cfrp_shm_size()

/**
 * cfrp传输协议
 * 服务端与客户端必须要按照这个协议通讯
 */
struct cfrp_protocol {
  // 本次会话类型
  // 0x01: 正常传输
  // 0x02: 断开连接
  // 0x03: 会话异常
  // 0x04: 建立新连接
  // 0x05: 建立主连接
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
  struct {
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
struct cfrp_payload {
  // 协议头
  struct cfrp_protocol head;
  // 传输的数据
  char *data;
};

/**
 * 映射信息
 */
struct cfrp_mapping {
  char *addr;
  uint port;
  struct list_head list;
};

/**
 * 会话信息
 */
struct cfrp_session {
  char sid[SID_LEN];
  // 目标
  struct sock *sk_desc;
  // 源
  struct sock *sk_src;
  void *ptr;
  // 会话所在worker
  void *woker;
  struct list_head list;
};

struct sock_event {
  struct sock *sk;
  int events;
  struct list_head list;
};

struct cfrp_epoll {
  int efd;
  struct sock_event events;
};

struct worker_operating {
  void (*start)(void *);
  int (*kill)(void *, struct cfrp_session *);
};

struct cfrp_worker {
  int pid;
  void *ctx;
  uint counter;
  struct worker_operating *op;
  // epoll
  struct cfrp_epoll *epoll;
  struct cfrp_session sessions;
  struct list_head list;
};

struct cfrp_job {
  // 互斥锁
  struct cfrp_lock *lock;
  struct cfrp_worker wokers;
};

struct cfrp_lock {
  // 互斥锁
  int mutex;
  // 获得锁的对象
  void *aptr;
};

struct cfrp_server {
  struct sock *msk_pair;
  // 接受连接事件
  struct sock_event sock_accept;
  // 等待 session
  struct cfrp_session *wait_sessions;
};

struct cfrp_client {};

struct cfrp {
  // 上下文
  void *ctx;
  // 映射信息
  struct cfrp_mapping mappings;
  // 工作
  struct cfrp_job job;
  // 多路复用
  struct cfrp_epoll *epoll;
  // 服务端或客户端实体
  void *entry;
  // 共享内存
  void *shm;
};

struct cfrp_operating {
  int (*start)(struct cfrp *);
  int (*stop)(struct cfrp *);
  int (*reload)(struct cfrp *);
  int (*restart)(struct cfrp *);
  int (*kill)(struct cfrp_session *slist, char *sid);
};

struct cfrp_arg {
  int argc;
  char **argv;
};

struct cfrp_context {
  int pid;
  struct cfrp_arg arg;
  struct cfrp *frp;
  struct cfrp_operating *op;
};

typedef struct cfrp_context cfrps;
typedef struct cfrp_context cfrpc;

extern size_t cfrp_shm_size();

extern cfrps *make_cfrps(char *bind_addr, uint port, struct cfrp_mapping *mappings, int argc, char **argv);

extern cfrpc *make_cfrpc(char *client_addr, uint port, struct cfrp_mapping *mappings, int argc, char **argv);

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
 * 停止
 */
extern int cfrp_stop(struct cfrp_context *ctx);

extern int cfrp_procinit(struct cfrp_context *ctx, int argv, char **argc);

extern int cfrp_setprotitle(struct cfrp_context *ctx, char *name);

typedef struct cfrp_worker fworker_t;
typedef struct cfrp cfrp_t;
typedef struct cfrp_mapping fmapping_t;
typedef struct cfrp_session fsession_t;
typedef struct sock fsock_t;
typedef struct cfrp_job fjob_t;
typedef struct cfrp_context fctx_t;
typedef struct cfrp_server fserver_t;
#endif