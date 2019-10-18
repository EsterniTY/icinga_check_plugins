#ifndef IF_STATUS_H
#define IF_STATUS_H

#include <sys/types.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "utils.h"

#define OCTET_MAX_VALUE 0xFFFFFFFFFFFFFFFF
#define octet_delta(a, b) ((a)>(b) ? (OCTET_MAX_VALUE-a+b) : (b-a))
#define time_delta(a) (timeDelta ? (a) / timeDelta : 0)

typedef u_int8_t ifEntry8_t;
typedef u_int32_t ifEntry32_t;
typedef u_int64_t ifEntry64_t;

struct if_status_t {
    struct if_status_t   *next;           // 8
    mtime_t               microtime;      // 8
    oid                   id;             // 8
    size_t                name_len;       // 8
    size_t                alias_len;      // 8
    ifEntry64_t           speed;          // 8
    ifEntry64_t           inOctets;       // 8
    ifEntry64_t           outOctets;      // 8
    ifEntry64_t           inUcastPkts;    // 8
    ifEntry64_t           outUcastPkts;   // 8
    ifEntry64_t           inMcastPkts;    // 8
    ifEntry64_t           outMcastPkts;   // 8
    ifEntry32_t           inErrors;       // 4
    ifEntry32_t           outErrors;      // 4
    ifEntry8_t            adminState;     // 1
    ifEntry8_t            operState;      // 1
    char                 *name;           // VAR
    char                 *alias;          // VAR
};

#endif /* IF_STATUS_H */
