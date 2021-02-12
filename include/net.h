#ifndef __NET_H__
#define __NET_H__
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define SOCK_LISTEN_NUM 10

#define SOCK_ADDR_IN(port, host) { \
    .sin_family = AF_INET,         \
    .sin_port = htons(port),       \
    .sin_addr = {.s_addr = inet_addr(host)}};

struct sock
{
    int fd;
    int type;
    int port;
    char *host;
};

struct listener
{
    int efd;
    int listen_num;
    struct sock *sk;
};

typedef void (*__handler)(struct listener* lr, int size, struct epoll_event *ev);

extern struct sock *make_tcp(uint port, char *bind_addr);

extern struct sock *make_udp(uint port, char *bind_addr);

extern struct sock *make_tcp_connect(uint port, char *host);

extern struct sock *make_udp_connect(uint port, char *host);

extern struct listener make_listener(struct sock *sk);

extern int handler_listener(struct listener *lr, uint max_event, __handler handler);

extern int set_noblocking(int fd);

extern struct sock *sock_accept(struct sock *sk);

extern int sock_send(struct sock *sk, char *bytes, uint size);

extern int sock_recv(struct sock *sk, char *buff, uint buff_size);

extern int sock_flush(struct sock *sk);

extern int sock_close(struct sock *sk);

#endif