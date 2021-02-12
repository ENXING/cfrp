#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "types.h"
#include "logger.h"
#include "net.h"
#include "lib.h"

typedef struct sock __sock;

extern struct sock *__get_sock();

struct sock *__get_sock(int fd, uint port, char *host)
{
    __sock *sk = malloc(sizeof(__sock));
    sk->fd = fd;
    sk->port = port;
    sk->host = host;
    sk->type = AF_INET;
    return sk;
}

struct sock *make_tcp(uint port, char *bind_addr)
{
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        log_error("make tcp error! msg: %s", port, SYS_ERROR);
        return NULL;
    }
    struct sockaddr_in addr = SOCK_ADDR_IN(port, bind_addr);
    socklen_t len = sizeof(struct sockaddr);
    if (bind(fd, (struct sockaddr *)&addr, len) < 0)
    {
        log_error("make tcp bind error! %s:%d, msg: %s", bind_addr, port, SYS_ERROR);
        return NULL;
    }

    if (listen(fd, SOCK_LISTEN_NUM) < 0)
    {
        return NULL;
    }
    struct sock *sk = __get_sock(fd, port, bind_addr);
    log_debug("make tcp success! %s:%d#%d", bind_addr, port, fd);
    return sk;
}

struct sock *make_udp(uint port, char *bind_addr)
{
}

struct sock *make_tcp_connect(uint port, char *host)
{
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        log_error("connect to %s:%d failure, msg: %s", host, port, SYS_ERROR);
        return NULL;
    }
    struct sockaddr_in addr = SOCK_ADDR_IN(port, host);
    socklen_t len = sizeof(struct sockaddr);
    if (connect(fd, (struct sockaddr *)&addr, len) < 0)
    {
        log_error("connect to %s:%d failure, msg: %s", host, port, SYS_ERROR);
        return NULL;
    }
    __sock *sk = __get_sock(fd, port, host);
    log_error("connect to %s:%d success", host, port);
    return sk;
}

struct sock *make_udp_connect(uint port, char *host)
{
}

int set_noblocking(int fd)
{
    int flag = fcntl(fd, F_GETFL);
    if (fcntl(fd, F_SETFL, flag | O_NONBLOCK) == -1)
    {
        log_error("set no blocking failure!");
        return C_ERROR;
    }
    return C_SUCCESS;
}

struct sock *sock_accept(struct sock *sk)
{
    struct sockaddr_in addr;
    socklen_t len;
    int fd;
    if ((fd = accept(sk->fd, (struct sockaddr *)&addr, &len)) < 0)
    {
        log_error("accpet connect error! %s:%d#%d, msg: %s", sk->host, sk->port, sk->fd, SYS_ERROR);
        return NULL;
    }
    __sock *connect_sk = __get_sock(fd, ntohs(addr.sin_port), inet_ntoa(addr.sin_addr));
    log_debug("%s:%d#%d accept connect %s:%d#%d", sk->host, sk->port, sk->fd, connect_sk->host, connect_sk->port, fd);
    return connect_sk;
}

int sock_send(struct sock *sk, char *bytes, uint size)
{
}

int sock_recv(struct sock *sk, char *buff, uint buff_size)
{
}

int sock_flush(struct sock *sk)
{
}

int sock_close(struct sock *sk)
{
}

struct listener make_listener(struct sock *sk)
{
    
}

extern int handler_listener(struct listener *lr, uint max_event, __handler handler)
{
    log_debug("handler events");
    struct epoll_event ev[max_event];
    int num = -1;
    LOOP
    {
        if ((num = epoll_wait(lr->efd, ev, max_event, 10)) == -1)
        {
            log_debug("handler error");
            break;
        }
        else
        {
            log_info("handler...");
            // handler(lr, num, ev);
        }
    }
}