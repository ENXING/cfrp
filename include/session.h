#ifndef __SESSION_H__
#define __SESSION_H__
#include "cfrp.h"

extern int cfrp_sadd(struct cfrp_session *slist, struct cfrp_session *session);

extern struct cfrp_session *cfrp_sget(struct cfrp_session *slist, char *sid);

extern struct cfrp_session *cfrp_sdel(struct cfrp_session *slist, char *sid);

/**
 * 生成一个唯一会话Id
 */
extern char *cfrp_gensid(char *dest, size_t size);

#endif