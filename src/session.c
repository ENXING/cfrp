#include "session.h"
#include "lib.h"

static char chrs[] = {'a', 'b', 'c', 'd', 'e', 'f', 'j', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
                      's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

int cfrp_sadd(struct cfrp_session *slist, struct cfrp_session *session) {
  __non_null__(slist, C_ERROR);
  list_add(&session->list, &slist->list);
  return C_SUCCESS;
}

struct cfrp_session *cfrp_sget(struct cfrp_session *slist, char *sid) {
  struct list_head *entry;
  struct cfrp_session *sn; 
  return NULL;
}

struct cfrp_session *cfrp_sdel(struct cfrp_session *slist, char *sid) {
  struct cfrp_session *sn = cfrp_sget(slist, sid);
  __non_null__(sn, NULL);
  list_del(sn->list.prev, sn->list.next);
  sn->list.next = sn->list.prev = NULL;
  return sn;
}

char *cfrp_gensid(char *dest, size_t size) {
  __non_null__(dest, NULL);
  return dest;
}