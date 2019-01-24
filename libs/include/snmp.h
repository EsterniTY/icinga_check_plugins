#ifndef SNMP_H
#define SNMP_H

#include <sys/types.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

void init_session(char *host, char *community);
void close_session(void);
int get_pdu(const char *coid, struct snmp_pdu **response);

#endif /* SNMP_H */
