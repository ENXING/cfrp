#include "cfrp.h"
#include "net.h"
#include "logger.h"

int cfrp_verify(struct cfrp *frp, char *sid)
{
}

int cfrp_proto(struct cfrp *frp, struct cfrp_protocol *dest, char *sid)
{
    if (!cfrp_recv(frp, sid, dest, sizeof(struct cfrp_protocol)))
    {
        log_error("read proto fialure");
        return C_ERROR;
    }
    return C_SUCCESS;
}

int cfrp_recv(struct cfrp *frp, char *sid, void *buff, size_t size)
{
    struct cfrp_session *sn = cfrp_sget(frp, sid);
    if (!sn)
    {
        return C_ERROR;
    }
    if (!sock_recv(sn->sk, buff, size))
    {
        return C_ERROR;
    }
    return C_SUCCESS;
}

int cfrp_send(struct cfrp *frp, char *sid, void *data, size_t size)
{
    struct cfrp_session *sn = cfrp_sget(frp, sid);
    
    if (!sn)
    {
        return C_ERROR;
    }
    if (!sock_send(sn->sk, data, size))
    {
        return C_ERROR;
    }
    return C_SUCCESS;
}

int cfrp_tranform(struct cfrp *frp, char *dest_sid, char *src_sid, size_t size)
{
    if (!frp || !dest_sid || !src_sid)
    {
        log_warning("Operate on a null pointer!");
        return C_ERROR;
    }
    char buff[size];
    if (!cfrp_recv(frp, src_sid, &buff, size))
    {
        log_error("recv faliure!");
        return C_ERROR;
    }
    else if (!cfrp_send(frp, dest_sid, buff, size))
    {
        log_error("send failure!");
        return C_ERROR;
    }
    return C_SUCCESS;
}