#ifndef __LIST_H__
#define __LIST_H__

#define list_entry(ptr, type, member)                                                                                                                \
  ({                                                                                                                                                 \
    const typeof(((type *)0)->member) *__mptr = (ptr);                                                                                               \
    (type *)((char *)__mptr - ((unsigned long)&((type *)0)->member));                                                                                \
  })

#define list_foreach(pos, head) for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_foreach_entry(pos, head, member)                                                                                                        \
  for (pos = (list_entry((head)->next, typeof(*pos), member)); &pos->member != (head); pos = (list_entry((&pos->member)->next, typeof(*pos), member)))

struct list_head {
  struct list_head *prev;
  struct list_head *next;
};

static inline void INIT_LIST_HEAD(struct list_head *list) {
  list->next = list;
  list->prev = list;
}

static inline void __list_add(struct list_head *new_list, struct list_head *prev, struct list_head *next) {
  next->prev     = new_list;
  new_list->next = next;
  new_list->prev = prev;
  prev->next     = new_list;
}

static inline void list_add(struct list_head *new_list, struct list_head *head) {
  __list_add(new_list, head, head->next);
}

static inline void list_del(struct list_head *prev, struct list_head *next) {
  prev->next = next;
  next->prev = prev;
}
#endif