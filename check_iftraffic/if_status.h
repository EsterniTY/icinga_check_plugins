#ifndef IF_STATUS_H
#define IF_STATUS_H

#include <sys/types.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "utils.h"

#define OCTET_MAX_VALUE 0xFFFFFFFFFFFFFFFF
#define octet_delta(a, b) ((a)>(b) ? (OCTET_MAX_VALUE-a+b) : (b-a))

typedef u_int8_t ifEntry8_t;
typedef u_int32_t ifEntry32_t;
typedef u_int64_t ifEntry64_t;

struct if_status_t {
    mtime_t microtime;   // 8
    oid id;              // 8
    ifEntry64_t speed;     // 8
    ifEntry64_t inOctets;  // 8
    ifEntry64_t outOctets; // 8
    ifEntry32_t inUcastPkts;  // 4
    ifEntry32_t outUcastPkts; // 4
    ifEntry8_t adminState; // 1
    ifEntry8_t operState;  // 1
    size_t name_len;     // 8
    char *name;          // VAR
    size_t alias_len;    // 8
    char *alias;         // VAR

    struct if_status_t *next;
};

#endif /* IF_STATUS_H */
