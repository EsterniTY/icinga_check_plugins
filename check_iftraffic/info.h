#ifndef INFO_H
#define INFO_H

#include <sys/types.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "if_status.h"

#define oid_ifSpeed32      5
#define oid_ifInOctets32   10
#define oid_ifOutOctets32  16

#define oid_ifSpeed64      15
#define oid_ifInOctets64   6
#define oid_ifOutOctets64  10
#define oid_ifAdminStatus  7
#define oid_ifOperStatus   8

#define IF_ADMIN_UP 1
#define IF_OPER_UP  1

#define IF_SPEED_1MB       1000000
#define IF_SPEED_20MB      20000000

void add_info(struct if_status_t **root,
              oid id,
              char *name,
              size_t name_len,
              char *alias,
              size_t alias_len,
              ifEntry8_t adminState,
              ifEntry8_t operState,
              ifEntry64_t speed,
              ifEntry64_t inOctets,
              ifEntry64_t outOctets,
              ifEntry32_t inUcastPkts,
              ifEntry32_t outUcastPkts);
void free_info(struct if_status_t *cell);

#endif /* INFO_H */
