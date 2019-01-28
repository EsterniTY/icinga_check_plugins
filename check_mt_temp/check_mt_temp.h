#ifndef CHECK_MT_TEMP_H
#define CHECK_MT_TEMP_H

#include <sys/types.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "utils.h"

#define MODE_OK 0
#define MODE_CPU 1
#define MODE_HW 2
#define MODE_BOTH 3

typedef u_int8_t temp_t;
typedef char msg_t[40];
typedef char perf_t[50];

extern struct opt_s {
    char *host;
    char *community;
    u_int mode;
    temp_t hw_warn;
    temp_t hw_crit;
    temp_t cpu_warn;
    temp_t cpu_crit;
} options;

temp_t get_temp(const oid *oid, const size_t oid_length);
void parse_args(int argc, char *argv[]);
void print_help(void);
code_t check_temp(const temp_t temp,
                  const temp_t warn,
                  const temp_t crit,
                  const char *msg_str,
                  const char *perf_str,
                  msg_t *msg,
                  perf_t *perf);

#endif /* CHECK_MT_TEMP_H */

