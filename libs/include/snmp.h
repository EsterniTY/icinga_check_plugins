#ifndef CHECK_LIBS_SNMP_H
#define CHECK_LIBS_SNMP_H

#include <sys/types.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#define ERRSTAT_RETURN 0
#define ERRSTAT_EXIT   1

void init_session(char *host, char *community, long version);
void close_session(void);
int _get_pdu(int type,
             const oid *coid,
             const size_t coid_length,
             struct snmp_pdu **response,
             int max_repetitions);
int get_pdu(const oid *coid,
            const size_t coid_length,
            struct snmp_pdu **response);
int get_pdu_next(const oid *coid,
                 const size_t coid_length,
                 struct snmp_pdu **response);
int get_pdu_bulk(const oid *coid,
                 const size_t coid_length,
                 struct snmp_pdu **response,
                 int max_repetitions);

void set_response_errstat_exit(u_int8_t status);
long check_response_errstat(struct snmp_pdu *response);

#ifdef DEBUG
size_t _get_pdu_requests(void);
#endif

#endif /* CHECK_LIBS_SNMP_H */
