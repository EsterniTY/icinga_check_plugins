#ifndef IFSNMP_H
#define IFSNMP_H

#include <sys/types.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

u_int64_t ifEntry64(const oid coid1, const oid coid2);
u_int32_t ifEntry32(const oid coid1, const oid coid2);
u_int8_t ifEntry8(const oid coid1, const oid coid2);

char *ifEntryAlias(const oid coid);

#endif /* IFSNMP_H */
