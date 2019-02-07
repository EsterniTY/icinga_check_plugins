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

void add_info(struct if_status_t **root,
              oid id,
              char *name,
              size_t name_len,
              u_int8_t adminState,
              u_int8_t operState,
              u_int64_t speed,
              u_int64_t inOctets,
              u_int64_t outOctets);
void free_info(struct if_status_t *cell);
void fill_info(struct if_status_t **curr, const struct variable_list *vars,
               char *name, size_t name_len);


#endif /* INFO_H */
