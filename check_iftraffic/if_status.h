#ifndef IF_STATUS_H
#define IF_STATUS_H

#include <sys/types.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "utils.h"

struct if_status_t {
    mtime_t microtime;   // 8
    oid id;              // 8
    u_int64_t speed;     // 8
    u_int64_t inOctets;  // 8
    u_int64_t outOctets; // 8
    u_int8_t adminState; // 1
    u_int8_t operState;  // 1
    size_t name_len;     // 8
    char *name;          // VAR
    size_t alias_len;    // 8
    char *alias;         // VAR

    struct if_status_t *next;
};

#endif /* IF_STATUS_H */
