#include "types.h"

#define buffer_SCOPE_HEAP 0x02

struct buffer
{
    // 初始大小
    size_t init_size;
    // 总大小
    size_t total_size;
    // 已使用
    size_t use_size;
    // buffer 内存分配位置 stack heap
    scope_t scope;
    // 数据
    char *bytes;
};

extern struct buffer *make_buffer(size_t size);
/**
 * 添加一个字符
*/
extern int buffer_achr(struct buffer *buf, char chr);

/**
 * copy一个指针地址数据到buffer
*/
extern int buffer_aany(struct buffer *buf, void *any, size_t size);

/**
 * 添加一串字符
*/
extern int buffer_astr(struct buffer *buf, char *str, size_t size);

/**
 * 在指定位置插入字一个符
*/
extern int buffer_ichr(struct buffer *buf, size_t index, char chr);

/**
 * 在指定位置插入一串字符
*/
extern int buffer_istr(struct buffer *buf, size_t index, char *str, size_t size);

/**
 * copy指定指定大小数据
*/
extern int buffer_iany(struct buffer *buf, size_t index, void *any, size_t size);

extern int buffer_sub(struct buffer *buf, void *dest, size_t begin, size_t end);

extern int buffer_zero(struct buffer *buf);

extern int buffer_free(struct buffer *buf);

extern int buffer_resize(struct buffer *buf);
