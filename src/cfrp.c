#include "cfrp.h"
#include "lib.h"

#define __check_op__(ctx)                                                                                                                            \
  if (!check_operating(ctx))                                                                                                                         \
    return C_ERROR;



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