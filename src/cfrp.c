#include "cfrp.h"
#include "lib.h"

#define __check_op__(ctx)                                                                                                                            \
  if (!check_operating(ctx))                                                                                                                         \
    return C_ERROR;

size_t cfrp_shm_size() {
  static const size_t lock_size = sizeof(struct cfrp_lock);
  static const size_t wait_sock_size = sizeof(struct cfrp_session) * CFRP_MAX_WAIT_CONN_NUM;
  static const size_t sk_pair = sizeof(struct sock) * 2;
  return lock_size + wait_sock_size + sk_pair;
}

static inline int check_operating(fctx_t *ctx) {
  __non_null__(ctx, C_ERROR);
  __non_null__(ctx->op, C_ERROR);
  return C_SUCCESS;
};

/**
 * 启动
 */
int cfrp_start(struct cfrp_context *ctx) {
  __check_op__(ctx);
  __non_null__(ctx->op->start, C_ERROR);
  return ctx->op->start(ctx->frp);
}

/**
 * 重新加载
 */
int cfrp_reload(struct cfrp_context *ctx) {
  __check_op__(ctx);
  __non_null__(ctx->op->reload, C_ERROR);
  return ctx->op->reload(ctx->frp);
}

/**
 * 停止
 */
int cfrp_stop(struct cfrp_context *ctx) {
  __check_op__(ctx);
  __non_null__(ctx->op->stop, C_ERROR);
  return ctx->op->stop(ctx->frp);
}

int cfrp_procinit(struct cfrp_context *ctx, int argv, char **argc) {
  __non_null__(ctx, C_ERROR);
  return C_ERROR;
}

int cfrp_setprotitle(struct cfrp_context *ctx, char *name) {
  __non_null__(ctx, C_ERROR);
  return C_ERROR;
}