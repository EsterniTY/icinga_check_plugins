#include <stdio.h>
#include <stdlib.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "utils.h"
#include "snmp.h"

static struct snmp_session *ss;
static u_int8_t _response_errstat_exit = ERRSTAT_EXIT;
static long _errstat = SNMP_ERR_NOERROR;

long try_session(char *host, char *community, long version)
{
    long session_version = version;
    init_session(host, community, version);

    if (version != SNMP_VERSION_1) {
        oid theOid[] = { 1, 3, 6, 1, 2, 1, 1, 1 };
        struct snmp_pdu *response;
        struct snmp_pdu *pdu;
        int status;

        pdu = snmp_pdu_create(SNMP_MSG_GET);
        snmp_add_null_var(pdu, theOid, OID_LENGTH(theOid));
        status = snmp_synch_response(ss, pdu, &response);

        snmp_free_pdu(response);

        if (status != STAT_SUCCESS) {
            close_session();
            init_session(host, community, SNMP_VERSION_1);
#ifdef DEBUG
            puts("Fallback to SNMP_VERSION_1");
#endif
            session_version = SNMP_VERSION_1;
        }
    }

    return session_version;
}

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

void check_response_errstat(struct snmp_pdu *response)
{
    _errstat = response->errstat;

    if (response->errstat != SNMP_ERR_NOERROR) {
        snmp_free_pdu(response);

        if (_response_errstat_exit == ERRSTAT_EXIT) {
            char *msg;
            switch (_errstat) {
            case SNMP_ERR_NOSUCHNAME:
                msg = "No such name";
                break;
            default:
                msg = "Error communicating to host";
                break;
            }

            close_session();
            exit_error(EXIT_CRITICAL, msg);
        }
    }
}

long errstat()
{
    return _errstat;
}

void iterate_vars(oid *root, size_t root_length, long repetitions,
                  size_t (*cc)(struct variable_list*, size_t idx),
                  int (*ch)(int, struct snmp_pdu*))
{
    u_int8_t can_go_next = 1;
    size_t idx = 0;

    oid name[MAX_OID_LEN];
    size_t name_len = root_length;

    oid end[MAX_OID_LEN];
    size_t end_len = root_length;

    memmove(end, root, root_length * sizeof(oid));
    end[end_len-1]++;

    memmove(name, root, root_length * sizeof(oid));

    while (can_go_next) {
        struct snmp_pdu *pdu;
        struct snmp_pdu *response;
        struct variable_list *vars;
        int status;

        pdu = snmp_pdu_create(SNMP_MSG_GETBULK);
        pdu->non_repeaters = 0;
        pdu->max_repetitions = repetitions;

        snmp_add_null_var(pdu, name, name_len);
        status = snmp_synch_response(ss, pdu, &response);

        if (ch == NULL) {
            if (status != STAT_SUCCESS) {
                snmp_free_pdu(response);
                exit_error(EXIT_CRITICAL, (status == STAT_ERROR
                                           ? "SNMP response error"
                                           : "SNMP response timedout"));
            }

            if (response->errstat != SNMP_ERR_NOERROR) {
                snmp_free_pdu(response);
                return;
            }
        } else if (ch(status, response) != 0) {
            snmp_free_pdu(response);
            return;
        }

        for (vars = response->variables; vars; vars = vars->next_variable) {
            if (snmp_oid_compare(end, end_len,
                                 vars->name, vars->name_length) <= 0) {
                can_go_next = 0;
                continue;
            }

            if (cc != NULL)
                idx = cc(vars, idx);

            memmove((char *)name, (char *)vars->name,
                    vars->name_length * sizeof(oid));
            name_len = vars->name_length;
        }

        snmp_free_pdu(response);
    }
}
