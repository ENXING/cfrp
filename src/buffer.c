#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "logger.h"
#include "types.h"

#define CHECK_NULL(buf)                            \
    if (!buf || !buf->bytes)                       \
    {                                              \
        log_warning("Operate on a null pointer!"); \
        return C_ERROR;                            \
    }

#define CHECK_SPACE(buf, size)                                                      \
    if (buf->total_size == buf->use_size || buf->total_size - buf->use_size < size) \
    {                                                                               \
        BMALLOC(buf, size);                                                         \
    }

#define BMALLOC(buf, size)     \
    if (!__bmalloc(buf, size)) \
        return C_ERROR;

#define BUFFER_OFFSET(buf, offset) (buf->bytes + offset)


static int __bmalloc(struct buffer *buf, int size)
{
    void *tmp;
    size_t total_size = 0;
    if (!size && !buf->bytes)
    {
        total_size = buf->total_size;
        tmp = malloc(sizeof(char) * buf->total_size);
        memset(tmp, '\0', buf->total_size);
    }
    else if (!size && buf->total_size == buf->use_size)
    {
        total_size = buf->total_size * 2;
        tmp = realloc(buf->bytes, total_size);
    }
    else if (size > 0)
    {
        total_size = buf->total_size + size;
        tmp = realloc(buf->bytes, size);
    }
    if (!tmp)
        return 0;
    buf->bytes = tmp;
    buf->total_size = total_size;
    return C_SUCCESS;
}

struct buffer *make_buffer(size_t size)
{
    struct buffer *buf = malloc(sizeof(struct buffer) * size);
    if (!buf)
        return NULL;
    buf->total_size = buf->init_size = size;
    buf->use_size = 0;
    buf->scope = C_SCOPE_HEAP;
    if (!__bmalloc(buf, 0))
    {
        free(buf);
        return NULL;
    }
    return buf;
}

int buffer_zero(struct buffer *buf)
{
    CHECK_NULL(buf);
    if (buf->total_size == 0)
        return C_ERROR;
    memset(buf->bytes, '\0', buf->total_size);
    buf->use_size = 0;
    return C_SUCCESS;
}

int buffer_achr(struct buffer *buf, char chr)
{
    return buffer_aany(buf, &chr, sizeof(char));
}

int buffer_astr(struct buffer *buf, char *str, size_t size)
{
    return buffer_aany(buf, str, size);
}

int buffer_aany(struct buffer *buf, void *any, size_t size)
{
    CHECK_NULL(buf);
    CHECK_SPACE(buf, size);
    if (!memcpy(BUFFER_OFFSET(buf, buf->use_size), any, size))
        return C_ERROR;
    buf->use_size += size;
    return C_SUCCESS;
}

int buffer_sub(struct buffer *buf, void *dest, size_t begin, size_t end)
{
    CHECK_NULL(buf);
    if (!dest || (end && begin >= end) || buf->use_size < end || buf->use_size < begin)
        return C_ERROR;
    return memcpy(dest, BUFFER_OFFSET(buf, begin), end ? end - begin : buf->use_size) ? C_SUCCESS : C_ERROR;
}

int buffer_ichr(struct buffer *buf, size_t index, char chr)
{
    return buffer_iany(buf, index, &chr, sizeof(char));
}

int buffer_istr(struct buffer *buf, size_t index, char *str, size_t size)
{
    return buffer_iany(buf, index, str, size);
}

int buffer_iany(struct buffer *buf, size_t index, void *any, size_t size)
{
    CHECK_NULL(buf);
    CHECK_SPACE(buf, size);
    // 先将需要被插入的位置移动
    // 插入数据
    if (!memmove(BUFFER_OFFSET(buf, index + size), BUFFER_OFFSET(buf, index), buf->use_size - index) ||
        !memcpy(BUFFER_OFFSET(buf, index), any, size))
        return C_ERROR;
    buf->use_size += size;
    return C_SUCCESS;
}

int buffer_free(struct buffer *buf)
{
    CHECK_NULL(buf)
    free(buf->bytes);
    buf->bytes = NULL;
    if (buf->scope == C_SCOPE_HEAP)
        free(buf);
    return C_SUCCESS;
}

int buffer_resize(struct buffer *buf)
{
    CHECK_NULL(buf);
    free(buf->bytes);
    buf->bytes = malloc(sizeof(char) * buf->init_size);
    log_debug("buffe reset: %d => %d", buf->total_size, buf->init_size);
    buf->total_size = buf->init_size;
    return buffer_zero(buf);
}