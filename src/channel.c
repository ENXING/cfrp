#include "channel.h"
#include "lib.h"
#include "logger.h"

extern int cfrp_channel_send(struct cfrp_channel *ch, struct cfrp_cmsg *__msg) {

  struct msghdr msg;
  struct iovec iov[1];

#ifdef CFRP_HAVE_MSGHDR_MSG_CONTROL

  union {
    struct cmsghdr cm;
    char control[CMSG_SPACE(sizeof(int))];
  } cmsg;

  if (__msg->fd == -1) {
    msg.msg_control    = NULL;
    msg.msg_controllen = 0;
  } else {
    cfrp_memzero(&cmsg, sizeof(cmsg));
    msg.msg_control    = cmsg.control;
    msg.msg_controllen = sizeof(cmsg.control);
    cmsg.cm.cmsg_len   = CMSG_LEN(sizeof(int));
    cmsg.cm.cmsg_level = SOL_SOCKET;
    cmsg.cm.cmsg_type  = SCM_RIGHTS;
    cfrp_memcpy(CMSG_DATA(&cmsg.cm), &__msg->fd, sizeof(int));
  }
  msg.msg_flags = 0;

#else

  if (ch->fd == -1) {
    msg.msg_accrights     = NULL;
    msg.msg_accrights_len = 0;
  } else {
    msg.msg_accrights     = (char *)&__msg->fd;
    msg.msg_accrights_len = sizeof(int);
  }

#endif
  iov[0].iov_base = __msg;
  iov[0].iov_len  = sizeof(cfrp_cmsg_t);

  msg.msg_name    = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov     = iov;
  msg.msg_iovlen  = 1;

  if (sendmsg(ch->fd, &msg, 0) < 0) {
    log_error("channel send error %s", CFRP_SYS_ERROR);
    return C_ERROR;
  }
  return C_SUCCESS;
}

extern int cfrp_channel_recv(struct cfrp_channel *ch, struct cfrp_cmsg *__msg) {

  struct msghdr msg;
  struct iovec iov[1];
  int n;

#ifdef CFRP_HAVE_MSGHDR_MSG_CONTROL

  union {
    struct cmsghdr cm;
    char *control[CMSG_SPACE(sizeof(int))];
  } cmsg;

  msg.msg_control    = cmsg.control;
  msg.msg_controllen = sizeof(cmsg.control);

#else

  int fd;
  msg.accrights    = &fd;
  msg.accrightslen = sizeof(int);

#endif

  iov[0].iov_base = __msg;
  iov[0].iov_len  = sizeof(cfrp_cmsg_t);

  msg.msg_iov     = iov;
  msg.msg_iovlen  = 1;
  msg.msg_name    = NULL;
  msg.msg_namelen = 0;

  if ((n = recvmsg(ch->fd, &msg, 0)) < 0) {
    log_error("channel recv error. msg: %s", CFRP_SYS_ERROR);
    return C_ERROR;
  }

#ifdef CFRP_HAVE_MSGHDR_MSG_CONTROL

  if (cmsg.cm.cmsg_level == SOL_SOCKET && cmsg.cm.cmsg_type == SCM_RIGHTS) {
    if (cmsg.cm.cmsg_len != (socklen_t)CMSG_LEN(sizeof(int))) {
      log_error("channel recv sock error");
      return C_ERROR;
    }
    cfrp_memcpy(&__msg->fd, CMSG_DATA(&cmsg.cm), sizeof(int));
  }

#else

  if (msg.accrightslen == sizeof(int)) {
    __msg.fd = fd;
  }

#endif
  return n;
}