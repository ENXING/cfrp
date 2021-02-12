#include <stdlib.h>
#include "list.h"

__list *make_list()
{
    __list *var_list = malloc(sizeof(__list));
    var_list->size = 0;
    var_list->head = NULL;
    var_list->head = NULL;
    return var_list;
}

int list_push(__list *list, void *elem)
{
      
}

int list_remove_index(__list *list, int index)
{
    
}

int list_remove(__list *list, void *elem)
{
}

int list_get(__list *list, int index)
{
}

int list_insert(__list *list, int index)
{
}