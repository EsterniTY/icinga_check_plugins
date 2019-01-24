#ifndef CHECK_MT_CPU_H
#define CHECK_MT_CPU_H

#include <sys/types.h>

#include "utils.h"

typedef u_int8_t load_t;

void print_help(void);
load_t get_cpu_load(const char *oid);

#endif /* CHECK_MT_CPU_H */
