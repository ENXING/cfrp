#ifndef __NET_H__
#define __NET_H__
#include <sys/socket.h>
#include <arpa/inet.h>

#include "buffer.h"

#define SOCK_LISTEN_NUM 10

#define SOCK_ADDR_IN(port, host) { \
    .sin_family = AF_INET,         \
    .sin_port = htons(port),       \
    .sin_addr = {.s_addr = inet_addr(host)}};

struct sock
{
    // 文件描述符
    int fd;
    // 类型
    int type;
    // 端口号
    int port;
    // 主机地址
    char *host;
    // 缓存
    struct buffer *cache;
};

extern struct sock *make_tcp(uint port, char *bind_addr);

extern struct sock *make_udp(uint port, char *bind_addr);

extern struct sock *make_tcp_connect(uint port, char *host);

extern struct sock *make_udp_connect(uint port, char *host);

extern int set_noblocking(int fd);

extern struct sock *sock_accept(struct sock *sk);

extern int sock_send(struct sock *sk, char *bytes, uint size);

extern int sock_recv(struct sock *sk, char *buff, uint buff_size);

extern int sock_flush(struct sock *sk);

extern int sock_close(struct sock *sk);

#endif