#include "cfrp.h"
#include "cshm.h"
#include "job.h"
#include "lib.h"
#include "logger.h"
#include "mutex.h"
#include "net.h"
#include "nevent.h"
#include "protocol.h"
#include "session.h"

static size_t cfrp_shm_size() {
  static const size_t lock_size      = sizeof(struct cfrp_lock);
  static const size_t wait_sock_size = sizeof(struct cfrp_session) * CFRP_MAX_WAIT_CONN_NUM;
  static const size_t sk_pair        = sizeof(struct sock) * 2;
  static const size_t counter        = sizeof(struct cfrp_counter);
  return lock_size + wait_sock_size + sk_pair + counter;
}

static int cfrp_release_sock_event(struct sock_event *event_head) {
  __non_null__(event_head, C_ERROR);

  cfrp_event_t *entry;
  cfrp_sock_t *sk;

  list_foreach_entry(entry, &event_head->list, list) {
    sk = entry->entry.sk;
    log_debug("release sock event: %s:%d", sk->host, sk->port);
    sock_close(sk);
    cfrp_free(entry);
  }

  return C_SUCCESS;
}

static void cfrp_server_auth_client_and_build(cfrp_server_t *fs, cfrp_sock_t *sk) {
  cfrp_void_assert(!fs);
  cfrp_void_assert(!sk);

  char type;
  cfrp_stream_t *op = sk->op;

  if ((op->recv(sk, &type, sizeof(char))) < 0) {
    log_info("1.not the client. close %s:%d#%d", sk->host, sk->port, sk->fd);
    sock_close(sk);
    return;
  }

  if (type != CFRP_PTMAC) {
    log_info("2.not the client. close %s:%d#%d", sk->host, sk->port, sk->fd);
    sock_close(sk);
    return;
  }

  cfrp_memcpy(cfrp_pair_last(fs->sock_pair), sk, sizeof(cfrp_sock_t));
  cfrp_free(sk);
  log_info("connect auth success: %s:%d#%d", sk->host, sk->port, sk->fd);
}

static int cfrp_server_build_mapping_session(cfrp_server_t *fs, cfrp_worker_t *wk, cfrp_sock_t *sk) {
  log_debug("build mapping");
  return C_ERROR;
}

/**
 * 通知客户端建立一个连接
 */
static void cfrp_server_build_mapping_wait_session(cfrp_server_t *fs, cfrp_sock_t *sk) {
  char sid[SID_LEN];
  cfrp_sock_t *client_sk              = cfrp_pair_last(fs->sock_pair);
  struct cfrp_proto_build_session pbs = {.type = CFRP_PTMPC};

  if (sock_send(client_sk, &pbs, sizeof(struct cfrp_proto_build_session)) < 0) {
    log_info("build mapping wait session error");
    sock_close(sk);
    return;
  }

  if (fs->wcounter->counter >= fs->wcounter->max) {
    log_info("wait for the session to go out of scope");
    fs->wcounter--;
    cfrp_session_t *rsn = fs->wait_sessions + 1 + fs->wcounter->counter;
    if (!cshm_isnui(rsn, sizeof(cfrp_session_t))) {
      sock_close(rsn->sk_desc);
    }
  }

  cfrp_session_t sn = {.sk_desc = sk}, *ptr = fs->wait_sessions + 1 + fs->wcounter->counter;
  cfrp_memcpy(ptr, &sn, sizeof(cfrp_session_t));
  list_add(&ptr->list, &fs->wait_sessions->list);
  fs->wcounter->counter++;
}

/**
 * 处理连接事件
 */
static void cfrp_server_handler_accept(struct cfrp *frp, cfrp_sock_t *sk, cfrp_worker_t *wk) {
  cfrp_sock_t *conn_sk = sock_accept(sk);
  __non_null__(conn_sk, ;);
  // 设置非阻塞式IO
  cfrp_server_t *fs      = (cfrp_server_t *)frp->entry;
  cfrp_sock_t *client_sk = cfrp_pair_last(fs->sock_pair);
  int client_exists      = !cshm_isnui(client_sk, sizeof(cfrp_sock_t));

  if (fs->sock_pair == sk) {
    // 如果是server端与客户端通讯的sock那么判断客户端是否存在
    // 如果不存在尝试进程客户端验证
    // 否则的话说明是客户端发起映射请求,那么将会进行映射认证
    sock_recv_timeout(conn_sk, 5);
    sock_send_timeout(conn_sk, 5);
    if (client_exists) {
      //检查客户端是否是发起映射请求
      log_debug("create a mapping request");
      cfrp_server_build_mapping_session(fs, wk, conn_sk);
    } else {
      // 检查客户端是否是与服务建立通讯请求
      log_debug("create a build request");
      cfrp_server_auth_client_and_build(fs, conn_sk);
    }
  } else if (client_exists && !sock_noblocking(conn_sk)) {
    // 如果是访问的映射端口, 那么判断看是否已经与客户建立连接,
    // 如果没有将会关闭连接
    log_debug("build the mapping request");
    cfrp_server_build_mapping_wait_session(fs, conn_sk);
  } else {
    // 还没有客户端建立通讯关闭该连接
    log_info("cfrp client not ready. close remote: %s:%d#%d", conn_sk->host, conn_sk->port, conn_sk->fd);
    sock_close(conn_sk);
  }
}

/**
 * 接受连接事件
 */
static void cfrp_server_accept_or_rw(struct cfrp *frp, cfrp_worker_t *wk) {
  log_debug("accepting connection events");
  cfrp_server_t *server  = (cfrp_server_t *)frp->entry;
  cfrp_sock_t *client_sk = cfrp_pair_last(server->sock_pair);
  cfrp_event_t events[10], *event;
  log_debug("add read events to the event set");
  list_foreach_entry(event, &server->sock_accept.list, list) {
    cfrp_epoll_add(frp->epoll, event);
  }
  if (!cshm_isnui(client_sk, sizeof(cfrp_sock_t))) {
    cfrp_event_t ev;
    ev.entry.sk = client_sk;
    cfrp_epoll_add(frp->epoll, &ev);
  }
  if (cfrp_epoll_wait(frp->epoll, events, 10, 20000) < 0) {
    log_error("accepting connection events error");
  };
  list_foreach_entry(event, &events->list, list) {
    if (event->events & CFRP_EVENT_IN) {
      if (event->type == CFRP_CHANNEL) {
        log_debug("recv channel message...");
      } else {
        if (event->entry.sk == client_sk) {
          log_info("has client message");
        } else {
          cfrp_server_handler_accept(frp, event->entry.sk, wk); // 处理连接事件
        }
      }
    } else if (event->events & CFRO_EVENT_ERR) {
    }
  }
  cfrp_epoll_clear(frp->epoll); // 清空所有事件集
}

static void cfrp_handler_event(cfrp_worker_t *wk) {
  sleep(1);
}

static void cfrp_server_process_handler(cfrp_worker_t *wk) {
  cfrp_t *frp = (cfrp_t *)wk->ctx;
  LOOP {
    cfrp_mutex(frp, wk, cfrp_server_accept_or_rw);
    cfrp_handler_event(wk);
  }
}

static void init_cfrp_server(struct cfrp *frp) {
  cfrp_server_t *fs     = (cfrp_server_t *)frp->entry;
  cfrp_sock_t *msk      = cfrp_pair_first(fs->sock_pair);
  cfrp_event_t *ev      = (struct sock_event *)cfrp_malloc(sizeof(struct sock_event));
  ev->entry.sk          = msk;
  fs->wcounter->counter = 0;
  fs->wcounter->max     = CFRP_MAX_WAIT_CONN_NUM - 1;
  frp->lock->mutex      = CFRP_MUTEX_UNLOCK;

  set_noblocking(msk->fd);
  list_add(&ev->list, &fs->sock_accept.list);
}

static void cfrp_server_print_info(struct cfrp *frp) {
  cfrp_mapping_t *mp;
  cfrp_server_t *fs = (cfrp_server_t *)frp->entry;

  log_info("listen [::%d]", fs->sock_pair->port);

  list_foreach_entry(mp, &frp->mappings.list, list) {
    log_info("mapping listen [::%d]", mp->port);
  }
}

static int cfrp_server_start(struct cfrp *frp) {
  log_info("start the server");
  init_cfrp_server(frp);
  cfrp_start_worker_process(frp, cfrp_cpu, cfrp_server_process_handler);
  cfrp_server_print_info(frp);
  log_info("the server started successfully");

  LOOP {
    sleep(1);
  }
}

static int cfrp_server_kill(struct cfrp_session *slist, char *sid) {
  cfrp_session_t *sn = cfrp_sdel(slist, sid);
  __non_null__(sn, C_ERROR);
  return C_SUCCESS;
}

static int cfrp_server_stop(struct cfrp *frp) {
  return C_ERROR;
}

static int cfrp_server_restart(struct cfrp *frp) {
  return C_ERROR;
}

static int cfrp_server_reload(struct cfrp *frp) {
  return C_ERROR;
}

static cfrp_operating_t server_operating = {
    .start = cfrp_server_start, .restart = cfrp_server_restart, .stop = cfrp_server_stop, .kill = cfrp_server_kill};

static int cfrp_server_mapping_init(struct sock_event *event_head, struct cfrp_mapping *mappings) {
  __non_null__(event_head, 1);
  __non_null__(mappings, 1);

  int err = 0;
  cfrp_mapping_t *mp;
  cfrp_sock_t *sk;
  cfrp_event_t *ev;

  list_foreach_entry(mp, &mappings->list, list) {
    sk = make_tcp(mp->port, mp->addr);
    ev = cfrp_malloc(sizeof(struct sock_event));

    if (!sk || !ev) {
      err = 1;
      break;
    }

    ev->entry.sk = sk;
    list_add(&ev->list, &event_head->list);
    log_debug("mapping listen: [::%d]", sk->port);
  }

  if (err) {
    log_info("mapping init failure, release mapping");
    cfrp_release_sock_event(event_head);
  }

  return !err;
}

cfrps_t *make_cfrps(char *bind_addr, uint port, struct cfrp_mapping *mappings, int argc, char **argv) {
  cfrps_t *frps         = cfrp_malloc(sizeof(cfrps_t));
  cfrp_t *frp           = cfrp_malloc(sizeof(cfrp_t));
  cfrp_server_t *server = cfrp_malloc(sizeof(cfrp_server_t));
  cfrp_shm_t *shm       = make_cshm(cfrp_shm_size());
  cfrp_sock_t *msk      = make_tcp(port, bind_addr);
  cfrp_epoll_t *epoll   = cfrp_epoll_create();

  int err = 0;

  if (sock_port_reuse(msk, 1) < 0) {
    err = 0;
    log_error("sock reuse port error");
  }

  __null__(frps) {
    log_error("cfrps malloc failure!");
    err = 1;
  }

  __null__(frp) {
    log_error("cfrps malloc frp failure!");
    err = 1;
  }

  __null__(msk) {
    log_error("cfrps tcp create failure! %s", SYS_ERROR);
    err = 1;
  }

  __null__(shm) {
    log_error("cfrps share memory create failure! %s", SYS_ERROR);
    err = 1;
  }

  __null__(server) {
    log_error("cfrps server malloc failure!");
    err = 1;
  }

  __null__(epoll) {
    log_error("cfrps epoll create failure!");
    err = 1;
  }

  INIT_LIST_HEAD(&server->sock_accept.list);

  if (err || !cfrp_server_mapping_init(&server->sock_accept, mappings)) {
    cfrp_free(frp);
    cfrp_free(frps);
    sock_close(msk);
    cshm_release(shm);
    cfrp_epoll_close(epoll);
    return NULL;
  }

  server->sock_pair = cshm_alloc(shm, sizeof(cfrp_sock_t) * 2);
  cfrp_memcpy(server->sock_pair, msk, sizeof(cfrp_sock_t));
  cfrp_free(msk);

  server->wcounter      = cshm_alloc(shm, sizeof(struct cfrp_counter));
  server->wait_sessions = cshm_alloc(shm, sizeof(struct cfrp_session) * CFRP_MAX_WAIT_CONN_NUM);
  frp->lock             = cshm_alloc(shm, sizeof(struct cfrp_lock));
  frp->ctx              = frps;
  frp->entry            = server;
  frp->epoll            = epoll;

  INIT_LIST_HEAD(&frp->mappings.list);
  INIT_LIST_HEAD(&server->wait_sessions->list);

  cfrp_mapping_t *entry, *mp;

  list_foreach_entry(entry, &mappings->list, list) {
    mp = cfrp_malloc(sizeof(cfrp_mapping_t));
    cfrp_memcpy(mp, entry, sizeof(cfrp_mapping_t));
    list_add(&mp->list, &frp->mappings.list);
  }

  // cfrp context
  frps->frp = frp;
  frps->pid = getpid();
  frps->op  = &server_operating;

  return frps;
}
