#include <errno.h>
#include <string.h>
#include <sys/types.h>

#define CFRP_SYS_ERROR strerror(errno)

#define C_ERROR 0
#define C_SUCCESS 1

#define C_SCOPE_HEAP 0x01
#define C_SCOPE_STACK 0x00

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef unsigned int cfrp_uint_t;
typedef unsigned long cfrp_ulong_t;
typedef unsigned char cfrp_uint8_t;
typedef unsigned long int cfrp_size_t;
typedef char *cfrp_name;
