#include "stream.h"
#include "lib.h"
#include "logger.h"

typedef int (*__sock_stream_io__)(void *sk, void *buff, size_t size);
typedef int (*__sock_stream_op__)(void *sk);

static int *sock_brecv(struct sock *sk, void *buff, size_t size) {
  log_info("brecv");
  return C_ERROR;
}

static int *sock_bsend(struct sock *sk, void *data, size_t size) {
  log_info("bsend");
  return C_ERROR;
}

static int *sock_bflush(struct sock *sk) {
  return C_ERROR;
}

static int *sock_bclose(struct sock *sk) {
  log_debug("close sock: %s:%d#%d", sk->host, sk->port, sk->fd);
  shutdown(sk->fd, SHUT_RDWR);
  close(sk);
  cfrp_free(sk);
  return C_SUCCESS;
}

/**
 * 正常发送
 */
struct stream_operating *stream_base(sock_t *sk) {
  __non_null__(sk, NULL);
  static struct stream_operating op = {.send = (__sock_stream_io__)sock_brecv,
                                       .recv = (__sock_stream_io__)sock_bsend,
                                       .flush = (__sock_stream_op__)sock_bflush,
                                       .close = (__sock_stream_op__)sock_bclose};
  sk->op = &op;
  return &op;
}

/**
 * 带缓存发送
 */
struct stream_operating *stream_buffer(sock_t *sk, struct buffer *buf) {
  __non_null__(sk, NULL);
  return NULL;
}

/**
 * 分包发送
 */
struct stream_operating *stream_subpackage(sock_t *sk, size_t total, size_t package) {
  __non_null__(sk, NULL);
  return NULL;
}