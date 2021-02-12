#ifndef __LIST_H__
#define __LIST_H__

struct node
{
    void *ptr;
    struct node *next;
    struct node *prev;
};

struct list
{
    unsigned int size;
    struct node *head;
    struct node *tail;
};

typedef struct list __list;

typedef struct node __node;

extern __list *make_list();

extern int list_push(__list *list, void *elem);

extern int list_remove_index(__list *list, int index);

extern int list_remove(__list *list, void *elem);

extern int list_get(__list *list, int index);

extern int list_insert(__list *list, int index);

#endif