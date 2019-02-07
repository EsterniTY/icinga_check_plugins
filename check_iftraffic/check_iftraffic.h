#ifndef CHECK_IFTRAFFIC_H
#define CHECK_IFTRAFFIC_H

#include <sys/types.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "if_status.h"

extern struct opt_s {
    char *host;
    char *community;
    long version;
    u_int warn;
    u_int crit;
    char *cache_dir;
    char *cache_path;
    char *filter;
    char *pattern;
    u_int downstate;
} options;

extern struct if_status_t if_status[];

void parse_args(int argc, char *argv[]);
void print_help(void);

struct if_status_t *load_snmp_info(void);
size_t str_format(char **result, const char *subject,
                  const char *pattern, char *format);

#endif /* CHECK_IFTRAFFIC_H */
