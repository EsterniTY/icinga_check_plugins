#ifndef CHECK_MT_CPU_H
#define CHECK_MT_CPU_H

#include <sys/types.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "utils.h"

typedef u_int8_t load_t;

void print_help(void);
load_t get_cpu_load(const oid *oid, const size_t oid_size);

#endif /* CHECK_MT_CPU_H */
