#ifndef __NET_H__
#define __NET_H__
#include <arpa/inet.h>
#include <sys/socket.h>

#include "buffer.h"

#define SOCK_LISTEN_NUM 10

#define SOCK_IPV4 0x01 // ipv4
#define SOCK_IPV6 0x02 // ipv6

#define SOCK_LEN_IPV4 sizeof(char) * 16
#define SOCK_LEN_IPV6 sizeof(char) * 40

#define SOCK_ADDR_IN(port, host) {.sin_family = AF_INET, .sin_port = htons(port), .sin_addr = {.s_addr = inet_addr(host)}};

#define SOCK_ADDR(sock) sock->atype == SOCK_IPV4 ? sock->addr.ipv4 : sock->addr.ipv6

#define SOCK_BAD -1 // 坏的 sock

struct stream_operating {
  int (*recv)(void *sk, void *buffer, size_t size);
  int (*send)(void *sk, void *data, size_t size);
  int (*flush)(void *sk);
  int (*close)(void *sk);
};

struct cfrp_sock {
  // 文件描述符
  int fd;
  // 类型
  int type;
  // 端口号
  int port;
  // 地址类型
  char atype;
  // 主机地址
  union {
    char ipv4[SOCK_LEN_IPV4];
    char ipv6[SOCK_LEN_IPV6];
  } addr;
  // 是否锁定操作
  char op_lock;
  // 流操作指针
  void *stream_ptr;
  struct stream_operating *op;
  char recv_stat;
  char send_stat;
};

extern struct cfrp_sock *make_tcp(cfrp_uint_t port, char *bind_addr);

extern struct cfrp_sock *make_udp(cfrp_uint_t port, char *bind_addr);

extern struct cfrp_sock *make_tcp_connect(cfrp_uint_t port, char *host);

extern struct cfrp_sock *make_udp_connect(cfrp_uint_t port, char *host);

extern int set_noblocking(int fd);

extern struct cfrp_sock *sock_accept(struct cfrp_sock *sk);

extern int sock_send(struct cfrp_sock *sk, void *bytes, size_t size);

extern int sock_recv(struct cfrp_sock *sk, void *buff, size_t buff_size);

extern int sock_flush(struct cfrp_sock *sk);

extern int sock_close(struct cfrp_sock *sk);

/**
 * 非阻塞 IO
 */
extern int sock_noblocking(struct cfrp_sock *sk);

/**
 * 读取超时时间
 */
extern int sock_recv_timeout(struct cfrp_sock *sk, int timeout);

/**
 * 写入超时时间
 */
extern int sock_send_timeout(struct cfrp_sock *sk, int timeout);

/**
 * 端口复用
 */
extern int sock_port_reuse(struct cfrp_sock *sk, int *val);

typedef struct cfrp_sock cfrp_sock_t;

#endif