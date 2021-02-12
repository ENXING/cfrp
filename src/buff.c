#include <stdlib.h>
#include <string.h>
#include "buff.h"

#define CHECK_NULL(buf)      \
    if (!buf || !buf->bytes) \
        return 0;

#define CHECK_SPACE(buf, size)                                                      \
    if (buf->total_size == buf->use_size || buf->total_size - buf->use_size < size) \
    {                                                                               \
        BMALLOC(buf, size);                                                         \
    }

#define BMALLOC(buf, size)     \
    if (!__bmalloc(buf, size)) \
        return 0;

extern int __bmalloc(struct buff *buf, int size);

int __bmalloc(struct buff *buf, int size)
{
    void *tmp;
    if (!size && !buf->bytes)
    {
        tmp = malloc(sizeof(char) * buf->total_size);
        memset(tmp, '\0', buf->total_size);
    }
    else if (!size && buf->total_size == buf->use_size)
    {
        tmp = realloc(buf->bytes, buf->total_size * 2);
    }
    else if (size > 0)
    {
        tmp = realloc(buf->bytes, size);
    }
    if (!tmp)
        return 0;
    buf->bytes = buf;
    return 1;
}

struct buff *make_buff(size_t size)
{
    struct buff *buf = malloc(sizeof(struct buff) * size);
    if (!buf)
        return NULL;
    buf->total_size = size;
    buf->use_size = 0;
    buf->scope = C_SCOPE_HEAP;
    if (!__bmalloc(buf, 0))
    {
        free(buf);
        return NULL;
    }
    return buf;
}

int buff_zero(struct buff *buf)
{
    CHECK_NULL(buf);
    if (buf->total_size == 0)
        return 0;
    memset(buf->bytes, '\0', buf->total_size);
}

int buff_achr(struct buff *buf, char chr)
{
    CHECK_NULL(buf);
    CHECK_SPACE(buf, sizeof(chr));
    buf->bytes[buf->use_size++] = chr;
    return 1;
}

int buff_astr(struct buff *buf, char *str, size_t size)
{
    CHECK_NULL(buf);
    CHECK_SPACE(buf, size);
}

int buff_ichr(struct buff *buf, char chr)
{
    CHECK_NULL(buf);
    CHECK_SPACE(buf, sizeof(char));
}

int buff_istr(struct buff *buf, char *str, size_t size)
{
    CHECK_NULL(buf);
    CHECK_SPACE(buf, size);
}

int buff_free(struct buff *buf)
{
    CHECK_NULL(buf)
    free(buf->bytes);
    if (buf->scope == C_SCOPE_HEAP)
        free(buf);
}