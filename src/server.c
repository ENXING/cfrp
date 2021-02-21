#include "cfrp.h"
#include "cshm.h"
#include "job.h"
#include "lib.h"
#include "logger.h"
#include "mutex.h"
#include "net.h"
#include "nevent.h"
#include "session.h"

static int cfrp_server_client_verify(sock_t *sk) {
  char type = sk->fd;
  return C_ERROR;
}

static int cfrp_server_client_session(fworker_t *wk, sock_t *sk) {
  return C_ERROR;
}

/**
 * 通知客户端建立一个连接
 *
 */
static void notify_client_connect(struct cfrp *frp, struct cfrp_session *sn) {
}

static void handler_accept(struct cfrp *frp, sock_t *sk, fworker_t *wk) {
  fserver_t *server = (fserver_t *)frp->entry;
  sock_t *client_sk = sock_accept(sk);
  __non_null__(client_sk, ;);
  if (server->msk_pair == sk) {
    if (!cfrp_spair(server->msk_pair)) {
      if (!cfrp_server_client_verify(client_sk)) {
        log_error("verify failure!");
      }
    } else {
      // 通知客户端建立连接
      log_debug("main sock pairs");
    }
  } else if (!cshm_isnui(cfrp_spair(server->msk_pair), sizeof(sock_t))) {
    struct cfrp_session *session = server->wait_sessions + 1;
    session->sk_src = NULL;
    session->sk_desc = client_sk;
    cfrp_gensid(session->sid, SID_LEN);
    list_add(&session->list, &server->wait_sessions->list);
    // 通知客户端连接
    notify_client_connect(frp, session);
  } else {
    // 还没有客户端建立通讯关闭该连接
    log_info("cfrp client not ready. close remote: %s:%d", client_sk->host, client_sk->port);
    sock_close(client_sk);
  }
}

static void cfrp_server_accept(struct cfrp *frp, fworker_t *wk) {
  log_debug("accept sock");
  fserver_t *server = (fserver_t *)frp->entry;
  //  将所有fd加入事件集
  struct list_head *entry;
  struct sock_event *event;
  list_foreach(entry, &server->sock_accept.list) {
    event = list_entry(entry, struct sock_event, list);
    cfrp_epoll_add(frp->epoll, event->sk);
  }
  struct sock_event events[10], *ev;
  int num = 0;
  if ((num = cfrp_epoll_wait(frp->epoll, events, 10, 5000)) < 0) {
    log_error("epoll error");
  };
  list_foreach(entry, &events->list) {
    ev = list_entry(entry, struct sock_event, list);
    if (ev->events & CFRP_EVENT_IN) {
      handler_accept(frp, ev->sk, wk);
    }
  };
  // release all
  cfrp_epoll_clear(frp->epoll);
}

static void cfrp_handler_read_or_write(fworker_t *wk) {

  // log_debug("read or wirte");
  sleep(1);
}

static void cfrp_server_process_handler(fworker_t *wk) {
  cfrp_t *frp = (cfrp_t *)wk->ctx;
  LOOP {
    cfrp_mutex(frp, wk, cfrp_server_accept);
    cfrp_handler_read_or_write(wk);
  }
}

static void init_cfrp_server(struct cfrp *frp) {
  fserver_t *server = (fserver_t *)frp->entry;
  frp->job.lock->mutex = CFRP_MUTEX_UNLOCK;
  sock_t *msk = cfrp_mpair(server->msk_pair);
  set_noblocking(msk->fd);
  struct sock_event *ev = (struct sock_event *)cfrp_malloc(sizeof(struct sock_event));
  ev->sk = msk;
  list_add(&ev->list, &server->sock_accept.list);
}

static int cfrp_server_start(struct cfrp *frp) {
  log_debug("cfrp init process");
  init_cfrp_server(frp);
  cfrp_start_worker_process(frp, cfrp_server_process_handler);
  log_debug("cfrp process started!");
  struct cfrp_accept_sock *ask;
  struct list_head *entry;
  LOOP {
    sleep(1);
  }
}

static int cfrp_server_kill(struct cfrp_session *slist, char *sid) {
  struct cfrp_session *sn = cfrp_sdel(slist, sid);
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

static struct cfrp_operating server_operating = {
    .start = cfrp_server_start, .restart = cfrp_server_restart, .stop = cfrp_server_stop, .kill = cfrp_server_kill};

static int cfrp_server_mapping_init(struct list_head *head, struct cfrp_mapping *mappings) {
  __non_null__(head, 1);
  __non_null__(mappings, 1);
  struct list_head *entry;
  struct cfrp_mapping *mp;
  struct sock *sk;
  struct sock_event *ev;
  int err = 0;
  list_foreach(entry, &mappings->list) {
    mp = list_entry(entry, struct cfrp_mapping, list);
    sk = make_tcp(mp->port, mp->addr);
    ev = cfrp_malloc(sizeof(struct sock_event));
    if (!sk || !ev) {
      err = 1;
      break;
    }
    ev->sk = sk;
    list_add(&ev->list, head);
    log_debug("mapping listen: [::%d]", sk->port);
  }
  if (!err) {
    return 1;
  }
  list_foreach(entry, head) {
    ev = list_entry(entry, struct sock_event, list);
    sock_close(sk);
  }
  return 0;
}

cfrps *make_cfrps(char *bind_addr, uint port, struct cfrp_mapping *mappings, int argc, char **argv) {
  int err = 0;
  cfrps *frps = cfrp_malloc(sizeof(cfrps));
  cfrp_t *frp = cfrp_malloc(sizeof(cfrp_t));
  struct sock *msk = make_tcp(port, bind_addr);
  fshm_t *shm = make_cshm(CFRP_SHMSIZE);
  struct cfrp_server *server = cfrp_malloc(sizeof(struct cfrp_server));
  struct cfrp_epoll *epoll = cfrp_epoll_create();
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
  if (err || !cfrp_server_mapping_init(&server->sock_accept.list, mappings)) {
    cfrp_free(frp);
    cfrp_free(frps);
    sock_close(msk);
    cshm_release(shm);
    cfrp_epoll_close(epoll);
    return NULL;
  }
  server->msk_pair = cshm_alloc(shm, sizeof(sock_t) * 2);
  cfrp_memcopy(server->msk_pair, msk, sizeof(sock_t));
  free(msk);
  server->wait_sessions = cshm_alloc(shm, sizeof(struct cfrp_session) * CFRP_MAX_WAIT_CONN_NUM);
  frp->job.lock = cshm_alloc(shm, sizeof(struct cfrp_lock));
  frp->ctx = frps;
  frp->entry = server;
  frp->epoll = epoll;
  // cfrp context
  frps->frp = frp;
  frps->pid = getpid();
  frps->op = &server_operating;
  INIT_LIST_HEAD(&server->wait_sessions->list);
  return frps;
}
