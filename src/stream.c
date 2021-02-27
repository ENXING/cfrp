#include "stream.h"
#include "lib.h"
#include "logger.h"

typedef int (*__sock_stream_io__)(void *sk, void *buff, size_t size);
typedef int (*__sock_stream_op__)(void *sk);

static int sock_brecv(struct cfrp_sock *sk, void *buff, size_t size) {
  log_info("sock recv %ld", size);
  return recv(sk->fd, buff, size, MSG_NOSIGNAL);
}

static int sock_bsend(struct cfrp_sock *sk, void *data, size_t size) {

  int n = send(sk->fd, data, size, MSG_NOSIGNAL);

  return n;
}

static int sock_bflush(struct cfrp_sock *sk) {
  return C_ERROR;
}

static int sock_bclose(struct cfrp_sock *sk) {
  log_debug("close sock: %s:%d#%d", SOCK_ADDR(sk), sk->port, sk->fd);
  shutdown(sk->fd, SHUT_RDWR);
  close(sk->fd);
  cfrp_free(sk);
  return C_SUCCESS;
}

/**
 * 正常发送
 */
struct stream_operating *stream_base(cfrp_sock_t *sk) {
  __non_null__(sk, NULL);
  static struct stream_operating op = {.send  = (__sock_stream_io__)sock_bsend,
                                       .recv  = (__sock_stream_io__)sock_brecv,
                                       .flush = (__sock_stream_op__)sock_bflush,
                                       .close = (__sock_stream_op__)sock_bclose};
  sk->op                            = &op;
  return &op;
}

/**
 * 带缓存发送
 */
struct stream_operating *stream_buffer(cfrp_sock_t *sk, struct buffer *buf) {
  __non_null__(sk, NULL);
  return NULL;
}

/**
 * 分包发送
 */
struct stream_operating *stream_subpackage(cfrp_sock_t *sk, size_t total, size_t package) {
  __non_null__(sk, NULL);
  return NULL;
}