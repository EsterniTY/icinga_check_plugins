#ifndef CHECK_MT_CPU_H
#define CHECK_MT_CPU_H

#include <sys/types.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "utils.h"

typedef u_int8_t load_t;
typedef char msg_t[40];
typedef char perf_t[50];

extern struct opt_s {
    char *host;
    char *community;
    load_t warn;
    load_t crit;
} options;

void parse_args(int argc, char *argv[]);
void print_help(void);
load_t get_cpu_load(const oid *oid, const size_t oid_size);

#endif /* CHECK_MT_CPU_H */
