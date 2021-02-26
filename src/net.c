#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <unistd.h>

#include "config.h"
#include "lib.h"
#include "logger.h"
#include "net.h"
#include "stream.h"
#include "types.h"

#define LOCAL_ADDR(addr) addr ? addr : CFRP_SOCK_ADDR

typedef struct cfrp_sock cfrp_sock_t;

#define __check_sock_op__(sk)                                                                                                                        \
  if (check_sock_op(sk) != C_SUCCESS) {                                                                                                              \
    log_warning("sock operating using null pointers");                                                                                               \
    return SOCK_BAD;                                                                                                                                 \
  }

static inline int check_sock_op(cfrp_sock_t *sk) {
  __non_null__(sk, C_ERROR);
  __non_null__(sk->op, C_ERROR);
  return C_SUCCESS;
}

static struct cfrp_sock *get_sock(int fd, cfrp_uint_t port, char *host, char atype) {

  cfrp_sock_t *sk = cfrp_malloc(sizeof(cfrp_sock_t));

  cfrp_memzero(sk, sizeof(cfrp_sock_t));

  sk->fd    = fd;
  sk->port  = port;
  sk->atype = atype;
  sk->type  = AF_INET;

  if (atype == SOCK_IPV4) {
    cfrp_strcpy(sk->addr.ipv4, host);
  } else if (atype == SOCK_IPV6) {
    cfrp_strcpy(sk->addr.ipv6, host);
  }

  stream_base(sk);

  return sk;
}

static inline int reuse_port(int fd, int *val) {
  return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)val, sizeof(int));
};

struct cfrp_sock *make_tcp(cfrp_uint_t port, char *__bind_addr) {
  int fd;
  int val         = 1;
  char *bind_addr = LOCAL_ADDR(__bind_addr);

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    log_error("make tcp error! msg: %s", port, CFRP_SYS_ERROR);
    return NULL;
  }

  struct sockaddr_in addr = SOCK_ADDR_IN(port, bind_addr);
  socklen_t len           = sizeof(struct sockaddr);

  if (reuse_port(fd, &val) < 0 || bind(fd, (struct sockaddr *)&addr, len) < 0 || listen(fd, SOCK_LISTEN_NUM) < 0) {
    close(fd);
    log_error("make tcp error! %s:%d, msg: %s", bind_addr, port, CFRP_SYS_ERROR);
    return NULL;
  }

  struct cfrp_sock *sk = get_sock(fd, port, bind_addr, SOCK_IPV4);

  log_debug("make tcp success! %s:%d#%d", bind_addr, port, fd);

  return sk;
}

struct cfrp_sock *make_udp(cfrp_uint_t port, char *bind_addr) {

  return NULL;
}

struct cfrp_sock *make_tcp_connect(cfrp_uint_t port, char *host) {
  int fd;
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    log_error("connect to %s:%d failure, msg: %s", host, port, CFRP_SYS_ERROR);
    return NULL;
  }
  struct sockaddr_in addr = SOCK_ADDR_IN(port, host);
  socklen_t len           = sizeof(struct sockaddr);
  if (connect(fd, (struct sockaddr *)&addr, len) < 0) {
    log_error("connect to %s:%d failure, msg: %s", host, port, CFRP_SYS_ERROR);
    return NULL;
  }
  cfrp_sock_t *sk = get_sock(fd, port, host, SOCK_IPV4);
  log_debug("connect to %s:%d success", host, port);
  return sk;
}

struct cfrp_sock *make_udp_connect(cfrp_uint_t port, char *host) {
  return NULL;
}

int set_noblocking(int fd) {
  int flag = fcntl(fd, F_GETFL);
  if (fcntl(fd, F_SETFL, flag | O_NONBLOCK) == -1) {
    log_error("set no blocking failure. msg: %s", CFRP_SYS_ERROR);
    return C_ERROR;
  }
  return C_SUCCESS;
}

struct cfrp_sock *sock_accept(struct cfrp_sock *sk) {

  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);

  int fd;
  if ((fd = accept(sk->fd, (struct sockaddr *)&addr, &len)) < 0) {
    log_error("accpet connect error! %s:%d#%d, msg: %s", SOCK_ADDR(sk), sk->port, sk->fd, CFRP_SYS_ERROR);
    return NULL;
  }

  cfrp_sock_t *conn_sk = get_sock(fd, ntohs(addr.sin_port), inet_ntoa(addr.sin_addr), SOCK_IPV4);
  log_debug("%s:%d#%d accept connect %s:%d#%d", SOCK_ADDR(sk), sk->port, sk->fd, SOCK_ADDR(conn_sk), conn_sk->port, fd);

  return conn_sk;
}

int sock_send(struct cfrp_sock *sk, void *bytes, size_t size) {
  __check_sock_op__(sk);
  __non_null__(sk->op->send, SOCK_BAD);
  return sk->op->send(sk, bytes, size);
}

int sock_recv(struct cfrp_sock *sk, void *buff, size_t buff_size) {
  __check_sock_op__(sk);
  __non_null__(sk->op->recv, SOCK_BAD);
  return sk->op->recv(sk, buff, buff_size);
}

int sock_flush(struct cfrp_sock *sk) {
  __check_sock_op__(sk);
  __non_null__(sk->op->flush, SOCK_BAD);
  return sk->op->flush(sk);
}

int sock_close(struct cfrp_sock *sk) {
  __check_sock_op__(sk);
  __non_null__(sk->op->close, SOCK_BAD);
  return sk->op->close(sk);
}

/**
 * 非阻塞 IO
 */
extern int sock_noblocking(struct cfrp_sock *sk) {
  __non_null__(sk, SOCK_BAD);
  return set_noblocking(sk->fd);
}

static inline int set_sock_timeout(struct cfrp_sock *sk, int timeout, int flag) {
  struct timeval tm = {.tv_sec = timeout, .tv_usec = 0};
  return setsockopt(sk->fd, SOL_SOCKET, flag, &tm, sizeof(struct timeval));
}

/**
 * 读取超时时间
 */
extern int sock_recv_timeout(struct cfrp_sock *sk, int timeout) {
  __non_null__(sk, SOCK_BAD);
  return set_sock_timeout(sk, timeout, SO_RCVTIMEO);
}

/**
 * 写入超时时间
 */
extern int sock_send_timeout(struct cfrp_sock *sk, int timeout) {
  __non_null__(sk, SOCK_BAD);
  return set_sock_timeout(sk, timeout, SO_SNDTIMEO);
}

/**
 * 端口复用
 */
extern int sock_port_reuse(struct cfrp_sock *sk, int *val) {
  __non_null__(sk, SOCK_BAD);
  return reuse_port(sk->fd, val);
}