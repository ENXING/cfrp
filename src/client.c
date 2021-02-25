#include "cfrp.h"
#include "cshm.h"
#include "job.h"
#include "lib.h"
#include "logger.h"
#include "mutex.h"

static size_t cfrp_shm_size() {
  static const size_t lock_size = sizeof(struct cfrp_lock);
  static const size_t sock_size = sizeof(cfrp_sock_t);
  return lock_size + sock_size;
}

static int cfrp_client_auth(cfrp_sock_t *sk) {
  char type = 0x05;
  if (sock_send(sk, &type, sizeof(char)) < 0) {
    log_error("send error");
    return C_ERROR;
  }
  return C_SUCCESS;
}

static void cfrp_client_check_connect(cfrp_client_t *fc) {
  cfrp_void_assert(!cshm_isnui(fc->csk, sizeof(cfrp_sock_t)));
  int try_cnn       = 0;
  cfrp_sock_t *conn = NULL;
  do {
    if (try_cnn) {
      log_info("retry the connection: %s:%d", fc->host, fc->port);
      sleep(5);
    }
    conn = make_tcp_connect(fc->port, fc->host);
    try_cnn++;
  } while (!conn);
  if (cfrp_client_auth(conn)) {
    log_info("connect to server: %s:%d", fc->host, fc->port);
    cfrp_memcpy(fc->csk, conn, sizeof(cfrp_sock_t));
  } else {
    // sock_close(conn);
  }
}

static void cfrp_client_main_connect_rw(struct cfrp *frp) {
  log_debug("wait server message");
  cfrp_client_t *fc = (cfrp_client_t *)frp->entry;
  cfrp_client_check_connect(fc);
  sleep(5);
};

static void cfrp_client_process_handler(struct cfrp *frp) {
  LOOP {
    cfrp_mutex(frp, cfrp_client_main_connect_rw);
    sleep(1);
  }
}

static int cfrp_client_start(struct cfrp *frp) {
  log_info("start the client");
  cfrp_start_worker_process(frp, cfrp_cpu, cfrp_client_process_handler, NULL);
  log_info("the client started successfully");
  LOOP {
    sleep(1);
  }
  return C_ERROR;
};

static int cfrp_client_stop(struct cfrp *frp) {
  return C_ERROR;
}

static int cfrp_client_restart(struct cfrp *frp) {
  return C_ERROR;
}

static int cfrp_client_reload(struct cfrp *frp) {
  return C_ERROR;
}

static struct cfrp_operating client_operating = {
    .start = cfrp_client_start, .stop = cfrp_client_stop, .reload = cfrp_client_reload, .restart = cfrp_client_restart};

cfrpc_t *make_cfrpc(char *client_addr, cfrp_uint_t port, struct cfrp_mapping *mappings, int argc, char **argv) {
  cfrpc_t *frpc        = (cfrpc_t *)cfrp_malloc(sizeof(cfrpc_t));
  cfrp_t *frp          = (struct cfrp *)cfrp_malloc(sizeof(cfrp_t));
  cfrp_client_t *entry = (cfrp_client_t *)cfrp_malloc(sizeof(cfrp_client_t));
  cfrp_shm_t *shm      = make_cshm(cfrp_shm_size());

  size_t host_len = sizeof(char) * cfrp_strlen(client_addr);
  entry->host     = cfrp_malloc(host_len);
  entry->port     = port;
  entry->csk      = cshm_alloc(shm, sizeof(cfrp_sock_t));

  cfrp_strcpy(entry->host, client_addr);
  INIT_LIST_HEAD(&entry->event_rw.list);

  frp->ctx   = frpc;
  frp->entry = entry;
  frp->lock  = cshm_alloc(shm, sizeof(struct cfrp_lock));
  frp->shm   = shm;

  frpc->frp = frp;
  frpc->op  = &client_operating;
  return frpc;
}
