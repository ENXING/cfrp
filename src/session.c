#include <stdlib.h>

#include "cfrp.h"

static char chrs[] = {
    'a', 'b', 'c', 'd', 'e', 'f', 'j',
    'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u',
    'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9'};

int cfrp_sadd(struct cfrp *frp, struct cfrp_session *session)
{
    session->head = frp->sessions.head;
    list_add(&session->list, session->head);
}

struct cfrp_session *cfrp_sget(struct cfrp *frp, char *sid)
{
    struct list_head *entry;
    struct cfrp_session *sn;
    list_foreach(entry, frp->sessions.head)
    {
        sn = list_entry(entry, struct cfrp_session, list);
        if (strcmp(sn->sid, sid) == 0)
            return sn;
    }
    return NULL;
}

struct cfrp_session *cfrp_sdel(struct cfrp *frp, char *sid)
{
    struct cfrp_session *sn = cfrp_sget(frp, sid);
    if (sn == NULL)
        return NULL;
    list_del(sn->list.prev, sn->list.next);
    sn->head = sn->list.next = sn->list.prev = NULL;
    return sn;
}

char *cfrp_gensid()
{

    return "";
}