#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <unistd.h>

#include "lib.h"
#include "logger.h"
#include "net.h"
#include "stream.h"
#include "types.h"

typedef struct sock sock_t;

#define __check_sock_op__(sk)                                                                                                                        \
  if (check_sock_op(sk) != C_SUCCESS)                                                                                                                \
    return C_ERROR;

static inline int check_sock_op(sock_t *sk) {
  __non_null__(sk, C_ERROR);
  __non_null__(sk->op, C_ERROR);
  return C_SUCCESS;
}

static struct sock *get_sock(int fd, uint port, char *host) {
  sock_t *sk = cfrp_malloc(sizeof(sock_t));
  sk->fd = fd;
  sk->port = port;
  sk->host = host;
  sk->type = AF_INET;
  stream_base(sk);
  return sk;
}

struct sock *make_tcp(uint port, char *bind_addr) {
  int fd;
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    log_error("make tcp error! msg: %s", port, SYS_ERROR);
    return NULL;
  }
  struct sockaddr_in addr = SOCK_ADDR_IN(port, bind_addr);
  socklen_t len = sizeof(struct sockaddr);
  if (bind(fd, (struct sockaddr *)&addr, len) < 0) {
    log_error("make tcp bind error! %s:%d, msg: %s", bind_addr, port, SYS_ERROR);
    return NULL;
  }

  if (listen(fd, SOCK_LISTEN_NUM) < 0) {
    return NULL;
  }
  struct sock *sk = get_sock(fd, port, bind_addr);
  log_debug("make tcp success! %s:%d#%d", bind_addr, port, fd);
  return sk;
}

struct sock *make_udp(uint port, char *bind_addr) {

  return NULL;
}

struct sock *make_tcp_connect(uint port, char *host) {
  int fd;
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    log_error("connect to %s:%d failure, msg: %s", host, port, SYS_ERROR);
    return NULL;
  }
  struct sockaddr_in addr = SOCK_ADDR_IN(port, host);
  socklen_t len = sizeof(struct sockaddr);
  if (connect(fd, (struct sockaddr *)&addr, len) < 0) {
    log_error("connect to %s:%d failure, msg: %s", host, port, SYS_ERROR);
    return NULL;
  }
  sock_t *sk = get_sock(fd, port, host);
  log_debug("connect to %s:%d success", host, port);
  return sk;
}

struct sock *make_udp_connect(uint port, char *host) {
  return NULL;
}

int set_noblocking(int fd) {
  int flag = fcntl(fd, F_GETFL);
  if (fcntl(fd, F_SETFL, flag | O_NONBLOCK) == -1) {
    log_error("set no blocking failure!");
    return C_ERROR;
  }
  return C_SUCCESS;
}

struct sock *sock_accept(struct sock *sk) {
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  int fd;
  if ((fd = accept(sk->fd, (struct sockaddr *)&addr, &len)) < 0) {
    log_error("accpet connect error! %s:%d#%d, msg: %s", sk->host, sk->port, sk->fd, SYS_ERROR);
    return NULL;
  }
  sock_t *connect_sk = get_sock(fd, ntohs(addr.sin_port), inet_ntoa(addr.sin_addr));
  log_debug("%s:%d#%d accept connect %s:%d#%d", sk->host, sk->port, sk->fd, connect_sk->host, connect_sk->port, fd);
  return connect_sk;
}

int sock_send(struct sock *sk, void *bytes, size_t size) {
  __check_sock_op__(sk);
  __non_null__(sk->op->send, C_ERROR);
  return sk->op->send(sk, bytes, size);
}

int sock_recv(struct sock *sk, void *buff, size_t buff_size) {
  __check_sock_op__(sk);
  __non_null__(sk->op->recv, C_ERROR);
  return sk->op->recv(sk, buff, buff_size);
}

int sock_flush(struct sock *sk) {
  __check_sock_op__(sk);
  __non_null__(sk->op->flush, C_ERROR);
  return sk->op->flush(sk);
}

int sock_close(struct sock *sk) {
  __check_sock_op__(sk);
  __non_null__(sk->op->close, C_ERROR);
  return sk->op->close(sk);
}

/**
 * 非阻塞 IO
 */
extern int sock_noblocking(struct sock *sk) {
  __non_null__(sk, C_ERROR);
  return set_noblocking(sk->fd);
}

static inline int set_sock_timeout(struct sock *sk, int timeout, int flag) {
  struct timeval tm = {.tv_sec = timeout, .tv_usec = 0};
  return setsockopt(sk->fd, SOL_SOCKET, flag, &tm, sizeof(struct timeval));
}

/**
 * 读取超时时间
 */
extern int sock_recv_timeout(struct sock *sk, int timeout) {
  __non_null__(sk, C_ERROR);
  return set_sock_timeout(sk, timeout, SO_RCVTIMEO);
}

/**
 * 写入超时时间
 */
extern int sock_send_timeout(struct sock *sk, int timeout) {
  __non_null__(sk, C_ERROR);
  return set_sock_timeout(sk, timeout, SO_SNDTIMEO);
}

/**
 * 端口复用
 */
extern int sock_port_reuse(struct sock *sk, int val) {
  __non_null__(sk, C_ERROR);
  return setsockopt(sk->fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));
}