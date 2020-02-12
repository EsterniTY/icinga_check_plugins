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

#define VENDOR_OTHER    0
#define VENDOR_MIKROTIK 1

#define CHECK_IDX if (idx >= _ifNumber) return idx

#define IF_ALLOC(name, size) \
    name = (size *)malloc(_ifNumber * sizeof(size)); \
    if (!name) \
        exit_error(EXIT_UNKNOWN, "Unable to allocate memory")

#define IF_ALLOC_8(name) IF_ALLOC(name, ifEntry8_t)
#define IF_ALLOC_32(name) IF_ALLOC(name, ifEntry32_t)
#define IF_ALLOC_64(name) IF_ALLOC(name, ifEntry64_t)

#define GET_COUNTER64() (((*vars->val.counter64).high << 32) + \
    (*vars->val.counter64).low)

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
    u_int    uptime;
} host_settings;

extern struct if_status_t if_status[];

void parse_args(int argc, char *argv[]);
void print_help(void);

struct if_status_t *load_snmp_info(void);
size_t str_format(char **result, const char *subject,
                  const char *pattern, char *format);

int check_percent(u_int threshold, struct delta_t *delta);

void add_msg(const struct if_status_t *item,
             char ***stack, const size_t count,
             const double in_p, const double out_p,
             const bytes_t in, const bytes_t out
             );

u_int get_host_uptime();
u_int8_t get_vendor();
#endif /* CHECK_IFTRAFFIC_H */
