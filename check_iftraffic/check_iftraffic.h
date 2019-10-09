#ifndef CHECK_IFTRAFFIC_H
#define CHECK_IFTRAFFIC_H

#include <sys/types.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "if_status.h"
#include "perfdata.h"

#define OVECCOUNT 30
#define NUMBER_IF_PREFIX "Interface "
#define IF_USAGE "Interface %s usage in: %.2f%%%s (%'lu bps), out: %.2f%%%s (%'lu bps)"
#define IF_USAGE_ALIAS "Interface %s (alias %s) usage in: %.2f%%%s (%'lu bps), out: %.2f%%%s (%'lu bps)"
#define MESSAGE_BUFER_SIZE 160

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
    u_long speed;
} options;

extern struct host_settings_s {
    u_int8_t has_ifSpeed64;
} host_settings;

typedef u_int64_t ifEntry64_t;
typedef u_int8_t ifEntry8_t;

extern struct if_status_t if_status[];

void parse_args(int argc, char *argv[]);
void print_help(void);

struct if_status_t *load_snmp_info(void);
size_t str_format(char **result, const char *subject,
                  const char *pattern, char *format);

void add_msg(const struct if_status_t *item,
             char ***stack, const size_t count,
             const double in_p, const double out_p,
             const bytes_t in, const bytes_t out
             );

#endif /* CHECK_IFTRAFFIC_H */
