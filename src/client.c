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

static size_t cfrp_shm_size() {
  static const size_t lock_size = sizeof(struct cfrp_lock);
  return lock_size;
}

static int cfrp_client_has_connect(struct cfrp *frp) {
  cfrp_client_t *client = frp->entry;

  return !cfrp_memiszero(&client->sock, sizeof(cfrp_sock_t));
}

static void cfrp_client_major_sync(struct cfrp *frp, struct cfrp_sock *sock) {
  cfrp_client_t *client = frp->entry;

  int sock_size = sizeof(cfrp_sock_t);

  if (cfrp_client_has_connect(frp)) {
    sock_close(&client->sock);
    cfrp_memzero(&client->sock, sock_size);
  } else {
    cfrp_memcpy(&client->sock, sock, sock_size);
  }
}

static void cfrp_command_handler(struct cfrp *frp, struct cfrp_sock *sock) {
  cfrp_client_t *client = frp->entry;
  cfrp_proto_command_t command;
  cfrp_session_t *session;
  cfrp_sock_t *dest, *src;
  cfrp_event_t events[2];

  log_debug("handler command");

  int n = sock_recv(sock, &command, sizeof(command));

  log_debug("....");

  if (n == 0) {
    // sock_close(sock);
  } else {

    log_debug("command cmd: %d, port: %d", command.cmd, command.port);

    if (command.cmd == CFRP_PTMPC) {
      log_debug("mapping");

      int port = command.port ? command.port : 22;

      dest = make_tcp_connect(22, "127.0.0.1");
      src  = make_tcp_connect(client->port, SOCK_ADDR(&client->sock));

      if (!dest || !src) {
        log_debug("connect error");
      }

      session = cfrp_malloc(sizeof(cfrp_session_t));

      session->sk_dest = dest;
      session->sk_src  = src;

      events[0].type     = CFRP_SOCK;
      events[0].entry.sk = dest;
      events[0].ptr      = session;

      events[1].type     = CFRP_SOCK;
      events[1].entry.sk = src;
      events[1].ptr      = session;

      sock_noblocking(dest);
      sock_noblocking(src);

      cfrp_epoll_add(frp->epoll_private, &events[0]);
      cfrp_epoll_add(frp->epoll_private, &events[1]);

      // list_add(&session->list, &frp->sessions.list);
    }
  }
}

static int cfrp_client_auth(cfrp_sock_t *sk) {
  char type = 0x05;
  if (sock_send(sk, &type, sizeof(char)) < 0) {
    log_error("send error");
    return C_ERROR;
  }
  return C_SUCCESS;
}

static void cfrp_client_check_connect(struct cfrp *frp) {
  if (cfrp_client_has_connect(frp))
    return;
  cfrp_client_t *client = frp->entry;

  int try_cnn       = 0;
  char *addr        = SOCK_ADDR(client);
  cfrp_sock_t *conn = NULL;

  do {

    if (try_cnn) {
      log_info("retry the connection: %s:%d", addr, client->port);
      sleep(5);
    }

    conn = make_tcp_connect(client->port, addr);
    try_cnn++;

  } while (!conn);

  if (cfrp_client_auth(conn)) {

    log_info("connect to server: %s:%d", addr, client->port);
    cfrp_memcpy(&client->sock, conn, sizeof(cfrp_sock_t));

    sock_noblocking(conn);
    cfrp_server_sync_sock(frp, conn, CFRP_CHANNEL_MASOCK);
    cfrp_free(conn);
  } else {
    sock_close(conn);
  }
}

static void cfrp_client_main_connect_read_and_write(struct cfrp *frp) {
  cfrp_client_check_connect(frp);

  int n;
  cfrp_event_t events[10], event, *ev;
  cfrp_client_t *client = frp->entry;
  cfrp_sock_t *sock     = &client->sock;

  event.type     = CFRP_SOCK;
  event.entry.sk = sock;
  event.ptr      = sock;

  cfrp_epoll_add(frp->epoll_share, &event);

  n = cfrp_epoll_wait(frp->epoll_share, events, 10, 1);

  list_foreach_entry(ev, &events->list, list) {
    if (ev->events & CFRP_EVENT_IN) {
      if (ev->type == CFRP_SOCK) {
        cfrp_command_handler(frp, ev->entry.sk);
      }
    }
  }
  cfrp_epoll_clear(frp->epoll_share);
};

static void cfrp_client_sock_event_handler(struct cfrp *frp, struct sock_event *event) {
  cfrp_sock_t *dest, *src;

  cfrp_session_t *session = event->ptr;

  dest = event->entry.sk;

  src = session->sk_dest == dest ? session->sk_src : session->sk_dest;

  sock_forward(dest, src);
}

static void cfrp_client_read_and_wirte(struct cfrp *frp) {
  cfrp_event_t events[10], *event;

  if (cfrp_epoll_wait(frp->epoll_private, events, 10, 1000) < 0) {
    log_error("epoll wait error msg: %s", CFRP_SYS_ERROR);
  }

  list_foreach_entry(event, &events->list, list) {
    if (event->type == CFRP_CHANNEL) {
      cfrp_channel_event_handler(frp, event->entry.channel);
    } else {
      log_info("sock event");
      cfrp_client_sock_event_handler(frp, event);
    }
  }
}

static void cfrp_display_info(struct cfrp *frp) {
  cfrp_unlock(frp->lock);
};

static void cfrp_client_process_handler(struct cfrp *frp) {
  log_info("start loop sock events");
  LOOP {
    cfrp_mutex(frp, cfrp_client_main_connect_read_and_write);
    cfrp_client_read_and_wirte(frp);
  }
}

static int cfrp_client_start(struct cfrp *frp) {
  log_info("start the client");
  cfrp_force_lock(frp->lock);
  cfrp_start_worker_process(frp, cfrp_cpu, cfrp_client_process_handler, cfrp_display_info);
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
  cfrp_t *frp                 = (cfrp_t *)cfrp_malloc(sizeof(cfrp_t));
  cfrpc_t *frpc               = (cfrpc_t *)cfrp_malloc(sizeof(cfrpc_t));
  cfrp_epoll_t *share_epoll   = cfrp_epoll_create();
  cfrp_epoll_t *private_epoll = cfrp_epoll_create();
  cfrp_client_t *client       = (cfrp_client_t *)cfrp_malloc(sizeof(cfrp_client_t));
  cfrp_shm_t *shm             = make_cshm(cfrp_shm_size());
  size_t host_len             = sizeof(char) * cfrp_strlen(client_addr);

  client->atype = SOCK_IPV4;
  client->port  = port;

  cfrp_strcpy(client->addr.ipv4, client_addr);
  cfrp_memzero(&client->sock, sizeof(cfrp_sock_t));

  frp->name          = "client";
  frp->ctx           = frpc;
  frp->entry         = client;
  frp->lock          = cshm_alloc(shm, sizeof(cfrp_lock_t));
  frp->shm           = shm;
  frp->epoll_private = private_epoll;
  frp->epoll_share   = share_epoll;
  frp->major_sync    = cfrp_client_major_sync;
  frpc->frp          = frp;
  frpc->op           = &client_operating;

  return frpc;
}
