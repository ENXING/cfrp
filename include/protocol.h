#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#define CFRP_PTMSG 0x01  // 发送数据
#define CFRP_PTDISC 0x02 // 断开连接
#define CFRP_PTEXC 0x03  // 会话异常
#define CFRP_PTMPC 0x04  // 建立映射连接
#define CFRP_PTMAC 0x05  // 建立主连接
#define CFRP_PTCX 0x06   // 关闭连接

struct cfrp_proto_build_session {
  char type;
  short slen;
  char *sid;
};

#endif