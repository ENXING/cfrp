#include "types.h"

#define BUFF_SCOPE_HEAP 0x02

struct buff
{
    // 初始大小
    size_t init_size;
    // 总大小
    size_t total_size;
    // 已使用
    size_t use_size;
    // buff 内存分配位置 stack heap
    scope_t scope;
    // 数据
    char *bytes;
};

extern struct buff *make_buff(size_t size);
/**
 * 添加一个字符
*/
extern int buff_achr(struct buff *buf, char chr);

/**
 * copy一个指针地址数据到buff
*/
extern int buff_aany(struct buff *buf, void *any, size_t size);

/**
 * 添加一串字符
*/
extern int buff_astr(struct buff *buf, char *str, size_t size);

/**
 * 在指定位置插入字一个符
*/
extern int buff_ichr(struct buff *buf, size_t index, char chr);

/**
 * 在指定位置插入一串字符
*/
extern int buff_istr(struct buff *buf, size_t index, char *str, size_t size);

/**
 * copy指定指定大小数据
*/
extern int buff_iany(struct buff *buf, size_t index, void *any, size_t size);

extern int buff_sub(struct buff *buf, void *dest, size_t begin, size_t end);

extern int buff_zero(struct buff *buf);

extern int buff_free(struct buff *buf);

extern int buff_resize(struct buff *buf);
