#ifndef __LIB_H__
#define __LIB_H__
#include <unistd.h>

#define LOOP for (;;)

#define cfrp_cpu sysconf(_SC_NPROCESSORS_ONLN)

#endif