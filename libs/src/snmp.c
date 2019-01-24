#include <stdio.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "snmp.h"
#include "utils.h"

static struct snmp_session *ss;

void init_session(char *host, char *community)
{
    struct snmp_session session;
    snmp_sess_init(&session);
    session.peername = host;
    session.version = SNMP_VERSION_2c;
    session.community = (u_char *) community;
    session.community_len = strlen(community);

    ss = snmp_open(&session);
}

void close_session(void)
{
    snmp_close(ss);
}

int get_pdu(const char *coid, struct snmp_pdu **response)
{
    struct snmp_pdu *pdu;
    int status;

    oid anOID[MAX_OID_LEN];
    size_t anOID_len = MAX_OID_LEN;

    pdu = snmp_pdu_create(SNMP_MSG_GET);

    read_objid(coid, anOID, &anOID_len);
    snmp_add_null_var(pdu, anOID, anOID_len);
    status = snmp_synch_response(ss, pdu, response);

    if (status != STAT_SUCCESS)
        exit_error(EXIT_CRITICAL, "Unable to connect to host");

    if ((*response)->errstat != SNMP_ERR_NOERROR)
        exit_error(EXIT_CRITICAL, "Error communicating to host");

    return status;
}
