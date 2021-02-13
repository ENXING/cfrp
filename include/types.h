#include <errno.h>
#include <string.h>

#define SYS_ERROR strerror(errno)

#define C_ERROR 0
#define C_SUCCESS 1

#define C_SCOPE_HEAP 0x01
#define C_SCOPE_STACK 0x00

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef char scope_t;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned char uint8;
