/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2021-01-28 20:13:39
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cfrpx/include/net.h
 */
#ifndef __NET_H__
#define __NET_H__
#include <sys/socket.h>
#include <sys/fcntl.h>

struct sock
{
    int fd;
    int port;
};

extern struct sock *make_tcp();

extern struct sock *make_udp();

extern int set_noblocking(int fd);

extern int sock_send(struct sock *sk, char *bytes, uint size);

extern int sock_recv(struct sock *sk, char *buff, uint buff_size);

extern int sock_flush(struct sock *sk);

extern int sock_close(struct sock *sk);

#endif