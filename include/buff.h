#include "types.h"

#define BUFF_SCOPE_HEAP 0x02

struct buff
{
    // 总大小
    size_t total_size;
    // 已使用
    size_t use_size;
    // buff 内存分配位置 stack heap
    scope_t scope;
    char *bytes;
};

extern struct buff *make_buff(size_t size);

extern int buff_achr(struct buff *buf, char chr);

extern int buff_aany(struct buff *buf, void *any, size_t size);

extern int buff_astr(struct buff *buf, char *str, size_t size);

extern int buff_ichr(struct buff *buf, char chr);

extern int buff_istr(struct buff *buf, char *str, size_t size);

extern int buff_iany(struct buff *buf, void *any, size_t size);

extern int buff_zero(struct buff *buf);

extern int buff_free(struct buff *buf);
