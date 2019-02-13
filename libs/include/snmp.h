#ifndef CHECK_LIBS_SNMP_H
#define CHECK_LIBS_SNMP_H

#include <sys/types.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#define ERRSTAT_RETURN 0
#define ERRSTAT_EXIT   1

long try_session(char *host, char *community, long version);
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
void check_response_errstat(struct snmp_pdu *response);
long errstat(void);
void iterate_vars(oid *root, size_t root_length, long repetitions,
                  size_t (*cc)(struct variable_list*, size_t idx),
                  int (*ch)(int, struct snmp_pdu*));

#endif /* CHECK_LIBS_SNMP_H */
