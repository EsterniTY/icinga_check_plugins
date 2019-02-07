#include "snmp.h"

#include "ifsnmp.h"

u_int64_t ifEntry64(const oid coid1, const oid coid2)
{
    oid theOid[] = { 1, 3, 6, 1, 2, 1, 31, 1, 1, 1, coid1, coid2 };

    struct snmp_pdu *response;
    u_int64_t value = 0;
    int status = 0;

    status = get_pdu(theOid, OID_LENGTH(theOid), &response);

    set_response_errstat_exit(ERRSTAT_RETURN);
    check_response_errstat(response);
    set_response_errstat_exit(ERRSTAT_EXIT);

    if (errstat() != SNMP_ERR_NOERROR)
        return 0;

    switch (response->variables->type) {
    case ASN_GAUGE:
        value = *response->variables->val.integer & 0xFFFFFFFF;
        break;
    case ASN_COUNTER64:
        value = ((*response->variables->val.counter64).high << 32) +
                (*response->variables->val.counter64).low;
        break;
    }

    snmp_free_pdu(response);

    return value;
}

u_int32_t ifEntry32(const oid coid1, const oid coid2)
{
    oid theOid[] = { 1, 3, 6, 1, 2, 1, 2, 2, 1, coid1, coid2 };

    struct snmp_pdu *response;
    u_int32_t value = 0;

    get_pdu(theOid, OID_LENGTH(theOid), &response);
    check_response_errstat(response);

    switch (response->variables->type) {
    case ASN_COUNTER:
    case ASN_GAUGE:
        value = (u_int32_t) *response->variables->val.integer;
        break;
    }

    snmp_free_pdu(response);

    return value;
}

u_int8_t ifEntry8(const oid coid1, const oid coid2)
{
    oid theOid[] = { 1, 3, 6, 1, 2, 1, 2, 2, 1, coid1, coid2 };

    struct snmp_pdu *response;
    u_int8_t value = 0;

    get_pdu(theOid, OID_LENGTH(theOid), &response);
    check_response_errstat(response);

    if (response->variables->type == ASN_INTEGER)
        value = (u_int8_t) *response->variables->val.integer & 0xFF;

    snmp_free_pdu(response);

    return value;
}

char *ifEntryAlias(const oid coid)
{
    oid theOid[] = { 1, 3, 6, 1, 2, 1, 31, 1, 1, 1, 1, coid };

    struct snmp_pdu *response;
    char *value = calloc(1, sizeof(char));

    get_pdu(theOid, OID_LENGTH(theOid), &response);
    check_response_errstat(response);

    if (response->variables->type == ASN_OCTET_STR) {
        size_t len = response->variables->val_len;
        value = realloc(value, len * sizeof(char));
        memcpy(value, response->variables->val.string, len);
    }

    snmp_free_pdu(response);

    return value;
}
