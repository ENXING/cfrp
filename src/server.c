#include "cfrp.h"
#include "channel.h"
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
  static const size_t lock_size = sizeof(struct cfrp_lock);
  return lock_size;
}

static void cfrp_server_major_sync(struct cfrp *frp, struct cfrp_sock *sock) {
  cfrp_server_t *server    = frp->entry;
  cfrp_sock_t *client_sock = cfrp_pair_last(server->sock_pair);
  if (!cfrp_memiszero(client_sock, sizeof(cfrp_sock_t))) {
    cfrp_channel_close(client_sock->fd);
    cfrp_memzero(client_sock, sizeof(cfrp_sock_t));
  } else {
    cfrp_memcpy(client_sock, sock, sizeof(cfrp_sock_t));
    log_info("proc channel sock pair %s:%d#%d", SOCK_ADDR(sock), sock->port, sock->fd);
  }
}

static void cfrp_clear_session(struct cfrp_session *sessions) {
  log_info("clear sessions");
  cfrp_session_t *session;
  list_foreach_entry(session, &sessions->list, list) {
    sock_close(session->sk_dest);
    sock_close(session->sk_src);
    list_del(session->list.prev, session->list.next);
    cfrp_free(session); // 这里可能会出现错误(保留)
  }
};

static int cfrp_release_sock_event(struct sock_event *event_head) {
  __non_null__(event_head, C_ERROR);

  cfrp_event_t *entry;
  cfrp_sock_t *sk;

  list_foreach_entry(entry, &event_head->list, list) {
    sk = entry->entry.sk;
    log_debug("release sock event: %s:%d", SOCK_ADDR(sk), sk->port);
    sock_close(sk);
    cfrp_free(entry);
  }

  return C_SUCCESS;
}

static void cfrp_server_auth_client_and_build(struct cfrp *frp, cfrp_sock_t *sk) {
  cfrp_void_assert(!sk);

  cfrp_server_t *server = (cfrp_server_t *)frp->entry;

  char type;

  if ((sock_recv(sk, &type, sizeof(char))) < 0) {
    log_info("1.not the client. close %s:%d#%d", SOCK_ADDR(sk), sk->port, sk->fd);
    sock_close(sk);
    return;
  }

  if (type != CFRP_PTMAC) {
    log_info("2.not the client. close %s:%d#%d", SOCK_ADDR(sk), sk->port, sk->fd);
    sock_close(sk);
    return;
  }

  cfrp_server_sync_sock(frp, sk, CFRP_CHANNEL_MASOCK);

  sock_noblocking(sk);

  cfrp_memcpy(cfrp_pair_last(server->sock_pair), sk, sizeof(cfrp_sock_t));

  log_info("connect auth success: %s:%d#%d", SOCK_ADDR(sk), sk->port, sk->fd);

  cfrp_free(sk);
}

static void cfrp_server_build_mapping_session(struct cfrp *frp, cfrp_sock_t *sk) {
  cfrp_server_t *server = (cfrp_server_t *)frp->entry;

  cfrp_session_t *session = NULL;

  cfrp_event_t event[2];

  log_info(".....");

  if (server->wait_session_num == -1) {
    log_debug("no session close connect");
    sock_close(sk);
  } else {
    // 认证是否是建立映射
    // 构建会话
    // 构建会话前通知其他进程关闭该会话
    log_info("forward");

    // cfrp_server_sync_sock(frp, sk, CFRP_CHANNEL_CLSN);

    session         = list_entry(server->wait_sessions.list.next, cfrp_session_t, list);
    session->sk_src = sk;

    event[0].type     = CFRP_SOCK;
    event[0].ptr      = session;
    event[0].entry.sk = session->sk_src;

    event[1].type     = CFRP_SOCK;
    event[1].ptr      = session;
    event[1].entry.sk = session->sk_dest;

    cfrp_epoll_add(frp->epoll_private, &event[0]);
    cfrp_epoll_add(frp->epoll_private, &event[1]);

    // list_del(session->list.prev, session->list.prev);
    // list_add(&session->list, &frp->sessions.list);
    server->wait_session_num--;
  }
}

/**
 * 通知客户端建立一个连接
 */
static void cfrp_server_build_mapping_wait_session(struct cfrp *frp, cfrp_sock_t *sk, cfrp_sock_t *src_sk) {
  cfrp_proto_command_t command = {.cmd = CFRP_PTMPC, .port = src_sk->port};
  cfrp_server_t *server        = frp->entry;
  cfrp_sock_t *client_sk       = cfrp_pair_last(server->sock_pair);
  cfrp_session_t *session      = cfrp_malloc(sizeof(cfrp_server_t));

  session->sk_dest = sk;
  session->sk_src  = NULL;

  int n = 1;

  // 通知客户端需要建立新的连接
  n = sock_send(client_sk, &command, sizeof(command));

  if (n <= 0) {
    log_info("build mapping wait session error");
    cfrp_free(session);
    sock_close(sk);
  } else {

    cfrp_server_sync_sock(frp, sk, CFRP_CHANNEL_OPSN);

    list_add(&session->list, &server->wait_sessions.list);

    server->wait_session_num++;
  }
}

/**
 * 处理连接事件
 */
static void cfrp_server_accept_handler(struct cfrp *frp, cfrp_sock_t *sk) {
  cfrp_sock_t *conn_sk = sock_accept(sk);
  cfrp_void_assert(!conn_sk);

  cfrp_server_t *fs      = (cfrp_server_t *)frp->entry;
  cfrp_sock_t *client_sk = cfrp_pair_last(fs->sock_pair);
  int client_exists      = !cfrp_memiszero(client_sk, sizeof(cfrp_sock_t));

  if (fs->sock_pair == sk) {
    // 如果是server端与客户端通讯的sock那么判断客户端是否存在
    // 如果不存在尝试进行客户端验证
    // 否则的话说明是客户端发起映射请求,那么将会进行映射认证

    if (client_exists) {
      // 检查客户端是否是发起映射请求
      // 如果映射队列为空那么拒绝该连接请求
      log_debug("create a mapping request");
      cfrp_server_build_mapping_session(frp, conn_sk);
    } else {
      // 检查客户端是否是与服务建立通讯请求
      log_info("create a build request");
      cfrp_server_auth_client_and_build(frp, conn_sk);
    }
  } else if (client_exists) {
    // 如果是访问的映射端口, 那么判断看是否已经与客户建立连接,
    // 如果没有将会关闭连接
    log_debug("build the mapping request");
    cfrp_server_build_mapping_wait_session(frp, conn_sk, sk);
  } else {
    // 还没有客户端建立通讯关闭该连接
    log_info("cfrp client not ready. close remote: %s:%d#%d", SOCK_ADDR(conn_sk), conn_sk->port, conn_sk->fd);
    sock_close(conn_sk);
  }
}

static void cfrp_server_client_handler(struct cfrp *frp) {

  cfrp_server_t *server    = (cfrp_server_t *)frp->entry;
  cfrp_sock_t *client_sock = cfrp_pair_last(server->sock_pair);
  cfrp_proto_command_t command;

  log_info("handler client message %s:%d#%d", SOCK_ADDR(client_sock), client_sock->port, client_sock->fd);

  cfrp_size_t size = sizeof(command);
  int n            = sock_recv(client_sock, &command, size);

  if (n == 0) {
    // 连接断开
    log_info("sock disconnect %s:%d#%d", SOCK_ADDR(client_sock), client_sock->port, client_sock->fd);

    cfrp_clear_session(&server->wait_sessions);
    cfrp_channel_close(client_sock->fd);
    cfrp_memzero(client_sock, sizeof(cfrp_sock_t));
  }
}

/**
 * 接受连接事件
 */
static void cfrp_server_accept_and_read_and_wirte(struct cfrp *frp) {
  log_debug("accepting connection events");

  cfrp_event_t events[10], *event;
  cfrp_server_t *server  = (cfrp_server_t *)frp->entry;
  cfrp_sock_t *client_sk = cfrp_pair_last(server->sock_pair);

  log_debug("add read events to the event set");

  list_foreach_entry(event, &server->sock_accept.list, list) {
    event->type = CFRP_SOCK;
    cfrp_epoll_add(frp->epoll_share, event);
  }

  if (!cfrp_memiszero(client_sk, sizeof(cfrp_sock_t))) {
    cfrp_event_t ev;
    ev.entry.sk = client_sk;
    ev.type     = CFRP_SOCK;
    cfrp_epoll_add(frp->epoll_share, &ev);
  }

  if (cfrp_epoll_wait(frp->epoll_share, events, 10, 1000) < 0) {
    log_error("accepting connection events error");
  };

  list_foreach_entry(event, &events->list, list) {
    if (event->events & CFRP_EVENT_IN) {
      if (event->entry.sk == client_sk) {
        cfrp_server_client_handler(frp);
      } else {
        cfrp_server_accept_handler(frp, event->entry.sk); // 处理连接事件
      }
    } else if (event->events & CFRO_EVENT_ERR) {
      log_debug("error");
    }
  }

  cfrp_epoll_clear(frp->epoll_share); // 清空所有事件集
}

static void cfrp_server_session_handler(struct cfrp *frp, struct sock_event *event) {
  // 开始处理转发代理数据

  char buffer[1024];
  int n, s;
  cfrp_session_t *session = event->ptr;
  cfrp_sock_t *dest;
  cfrp_sock_t *src;

  dest = event->entry.sk;

  src = session->sk_dest == dest ? session->sk_src : session->sk_dest;

  // 开始转发数据

  n = sock_forward(dest, src);

  if (!n || n < 0) {
    cfrp_epoll_del(frp->epoll_private, event);
    sock_close(dest);
    sock_close(src);
  }
}

static void cfrp_handler_event(cfrp_t *frp) {
  cfrp_event_t events[10], *event;

  if (cfrp_epoll_wait(frp->epoll_private, events, 10, 1000) < 0) {
    log_error("epoll error!");
  }

  list_foreach_entry(event, &events->list, list) {
    if (event->events & CFRP_EVENT_IN) {
      if (event->type == CFRP_CHANNEL) {
        cfrp_channel_event_handler(frp, event->entry.channel);
      } else if (event->type == CFRP_SOCK) {
        cfrp_server_session_handler(frp, event);
      } else {
        log_warning("bad sock ");
        cfrp_epoll_del(frp->epoll_share, event);
      }
    } else if (event->events & CFRO_EVENT_ERR) {
    }
  }
}

static void cfrp_server_process_handler(cfrp_t *frp) {

  LOOP {
    cfrp_mutex(frp, cfrp_server_accept_and_read_and_wirte);
    cfrp_handler_event(frp);
  }
}

static void init_cfrp_server(struct cfrp *frp) {
  cfrp_server_t *fs = (cfrp_server_t *)frp->entry;
  cfrp_sock_t *msk  = cfrp_pair_first(fs->sock_pair);
  cfrp_event_t *ev  = (struct sock_event *)cfrp_malloc(sizeof(struct sock_event));
  ev->entry.sk      = msk;

  frp->lock->mutex = CFRP_MUTEX_UNLOCK;

  set_noblocking(msk->fd);
  list_add(&ev->list, &fs->sock_accept.list);
}

static void cfrp_display_info(struct cfrp *frp) {
  cfrp_mapping_t *mapping;
  cfrp_worker_t *worker;
  cfrp_channel_t *channel;
  cfrp_server_t *server = (cfrp_server_t *)frp->entry;

  log_info("cfrp listen [::%d]", server->sock_pair->port);

  list_foreach_entry(mapping, &frp->mappings.list, list) {
    log_info("mapping listen [::%d]", mapping->listen_port);
  }

  list_foreach_entry(worker, &frp->workers.list, list) {
    channel = &frp->channels[worker->channel_solt];
    log_info("worker proc running %s[%d] cfrp.channels[%d] -- %d", frp->name, worker->pid, channel->slot, channel->fd);
  }

  cfrp_unlock(frp->lock);
}

static int cfrp_server_start(struct cfrp *frp) {
  pid_t wpid;
  int val = 0;
  cfrp_worker_t *wk;

  log_info("start the server");

  init_cfrp_server(frp);
  cfrp_force_lock(frp->lock);
  cfrp_start_worker_process(frp, cfrp_cpu, cfrp_server_process_handler, cfrp_display_info);
  log_info("the server started successfully");
  LOOP {
    // list_foreach_entry(wk, &frp->workers.list, list) {
    //   wpid = cfrp_waitpid(wk->pid, &val);
    //   if (wpid != 0) {
    //     list_del(wk->list.prev, wk->list.next);
    //     cfrp_unlock(frp->lock);
    //   }
    // }
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
    sk = make_tcp(mp->listen_port, mp->lisen_addr);
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

cfrps_t *make_cfrps(char *bind_addr, cfrp_uint_t port, struct cfrp_mapping *mappings, int argc, char **argv) {

  cfrp_mapping_t *mapping_entry, *mapping;

  cfrps_t *frps               = cfrp_malloc(sizeof(cfrps_t));
  cfrp_t *frp                 = cfrp_malloc(sizeof(cfrp_t));
  cfrp_server_t *server       = cfrp_malloc(sizeof(cfrp_server_t));
  cfrp_shm_t *shm             = make_cshm(cfrp_shm_size());
  cfrp_sock_t *sock           = make_tcp(port, bind_addr);
  cfrp_epoll_t *epoll_share   = cfrp_epoll_create();
  cfrp_epoll_t *epoll_private = cfrp_epoll_create();

  int err = 0;

  __null__(frps) {
    log_error("cfrps malloc failure!");
    err = 1;
  }

  __null__(frp) {
    log_error("cfrps malloc frp failure!");
    err = 1;
  }

  __null__(sock) {
    log_error("cfrps tcp create failure! %s", CFRP_SYS_ERROR);
    err = 1;
  }

  __null__(shm) {
    log_error("cfrps share memory create failure! %s", CFRP_SYS_ERROR);
    err = 1;
  }

  __null__(server) {
    log_error("cfrps server malloc failure!");
    err = 1;
  }

  __null__(epoll_share) {
    log_error("cfrps share epoll create failure!");
    err = 1;
  }

  __null__(epoll_private) {
    log_error("cfrps private epoll create failure!");
    err = 1;
  }

  INIT_LIST_HEAD(&server->sock_accept.list);
  INIT_LIST_HEAD(&server->wait_sessions.list);
  INIT_LIST_HEAD(&frp->mappings.list);

  if (err || !cfrp_server_mapping_init(&server->sock_accept, mappings)) {
    cfrp_free(frp);
    cfrp_free(frps);
    sock_close(sock);
    cshm_release(shm);
    cfrp_epoll_close(epoll_share);
    cfrp_epoll_clear(epoll_private);
    return NULL;
  }

  list_foreach_entry(mapping_entry, &mappings->list, list) {
    mapping = cfrp_malloc(sizeof(cfrp_mapping_t));
    cfrp_memcpy(mapping, mapping_entry, sizeof(cfrp_mapping_t));
    list_add(&mapping->list, &frp->mappings.list);
  }

  cfrp_memcpy(server->sock_pair, sock, sizeof(cfrp_sock_t));

  cfrp_free(sock);

  server->ctx        = frp;
  frp->name          = argv[0];
  frp->lock          = cshm_alloc(shm, sizeof(cfrp_lock_t));
  frp->ctx           = frps;
  frp->entry         = server;
  frp->epoll_share   = epoll_share;
  frp->epoll_private = epoll_private;
  frp->major_sync    = cfrp_server_major_sync;
  frp->worker        = NULL;
  frp->channel       = NULL;
  frps->frp          = frp;
  frps->pid          = getpid();
  frps->op           = &server_operating;

  return frps;
}
