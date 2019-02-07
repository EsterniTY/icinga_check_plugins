#include <stdio.h>
#include <stdlib.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "utils.h"
#include "snmp.h"

static struct snmp_session *ss;
static u_int8_t _response_errstat_exit = ERRSTAT_EXIT;

#ifdef DEBUG
static size_t _pdu_request_counter = 0;
#endif

void init_session(char *host, char *community, long version)
{
    struct snmp_session session;
    char *error;

    snmp_sess_init(&session);
    session.peername = host;
    session.version = version;
    session.community = (u_char *) community;
    session.community_len = strlen(community);

    ss = snmp_open(&session);

    if (ss == NULL) {
        snmp_error(&session, NULL, NULL, &error);
        exit_error(EXIT_CRITICAL, error);
    }
}

void close_session(void)
{
    if (ss)
        snmp_close(ss);
}

int _get_pdu(int type,
             const oid *coid,
             const size_t coid_length,
             struct snmp_pdu **response,
             int max_repetitions)
{
    struct snmp_pdu *pdu;
    int status;

#ifdef DEBUG
    _pdu_request_counter++;

    char *str = calloc(4096, sizeof(char));
    char p[10];
    size_t len = 0;
    for (int i = 0; i < (int)coid_length; i++) {
        snprintf(p, 10, ".%lu", coid[i]);
        len += strlen(p);
        strcat(str, p);
    }
    str[len] = '\0';
    printf("_get_pdu #%lu: %s\n", _pdu_request_counter, str);
#endif

    pdu = snmp_pdu_create(type);

    if (type == SNMP_MSG_GETBULK) {
        pdu->non_repeaters = 0;
        pdu->max_repetitions = max_repetitions;
    }

    snmp_add_null_var(pdu, coid, coid_length);
    status = snmp_synch_response(ss, pdu, response);

    if (status == STAT_TIMEOUT) {
        close_session();
        exit_error(EXIT_UNKNOWN, "Timed out. Check community string");
    }

    if (status != STAT_SUCCESS) {
        close_session();
        exit_error(EXIT_CRITICAL, "Unable to connect to host");
    }

    return status;
}

int get_pdu(const oid *coid,
            const size_t coid_length,
            struct snmp_pdu **response)
{
    return _get_pdu(SNMP_MSG_GET, coid, coid_length, response, 0);
}

int get_pdu_next(const oid *coid,
                 const size_t coid_length,
                 struct snmp_pdu **response)
{
     return _get_pdu(SNMP_MSG_GETNEXT, coid, coid_length, response, 0);
}

int get_pdu_bulk(const oid *coid,
                 const size_t coid_length,
                 struct snmp_pdu **response,
                 int max_repetitions)
{
    return _get_pdu(SNMP_MSG_GETBULK,
                    coid, coid_length, response, max_repetitions);
}

void set_response_errstat_exit(u_int8_t status)
{
    _response_errstat_exit = status;
}

long check_response_errstat(struct snmp_pdu *response)
{
    if (response->errstat != SNMP_ERR_NOERROR) {
        snmp_free_pdu(response);

        if (_response_errstat_exit == ERRSTAT_EXIT) {
            close_session();
            exit_error(EXIT_CRITICAL, "Error communicating to host");
        }
    }

    return response->errstat;
}

#ifdef DEBUG
size_t _get_pdu_requests(void)
{
    return _pdu_request_counter;
}
#endif
