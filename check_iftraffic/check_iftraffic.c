#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pcre.h>
#include <callback.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "snmp.h"
#include "utils.h"
#include "ifsnmp.h"
#include "if_status.h"
#include "info.h"
#include "perfdata.h"

#include "check_iftraffic.h"

extern const char *__progname;
static const char *__version = "1.0";

void parse_args(int argc, char *argv[])
{
    int opt;
    options.version = -1;

    while ((opt = getopt(argc, argv, "H:C:w:c:120hf:t:p:")) != -1)
    {
        switch (opt)
        {
        case 'H':
            options.host = optarg;
            break;
        case 'C':
            options.community = optarg;
            break;
        case 'w':
            options.warn = (u_int) atoi(optarg);
            break;
        case 'c':
            options.crit = (u_int) atoi(optarg);
            break;
        case '1':
            options.version = SNMP_VERSION_1;
            break;
        case '2':
            options.version = SNMP_VERSION_2c;
            break;
        case 'f':
            options.filter = optarg;
            break;
        case 'p':
            options.pattern = optarg;
            break;
        case '0':
            options.downstate = 1;
            break;
        case 't':
            options.cache_dir = optarg;
            break;
        case 'h':
            print_help();
            exit(0);
        }
    }

    if (options.host == NULL)
        exit_error(EXIT_CRITICAL, "No host defined");

    if (!options.community)
        options.community = "public";

    if (options.version == -1)
        options.version = SNMP_VERSION_2c;

    if (!options.filter)
        options.filter = NULL;

    if (!options.pattern)
        options.pattern = NULL;

    if (!options.downstate)
        options.downstate = 0;

    if (!options.cache_dir)
        options.cache_dir = "/tmp/mt";

    options.cache_path = malloc(128);
    sprintf(options.cache_path, "%s/%s.dat", options.cache_dir, options.host);
}

void print_help()
{
    printf("Check interface utilisation. ");
    printf("Version %s.\n\n", __version);
    printf("Usage: %s -H <host_address> [-C <community>] [-2]\n", __progname);
    puts("\t[-w <warning>] [-c <critical>]");
    puts("Options:");
    puts("\t-H    Host to check");
    puts("\t-C    SNMP community name ('public' is used if ommited)");
    puts("\t-1    Use SNMP version 1");
    puts("\t-2    Use SNMP version 2c");
    puts("\t-w    Optional warning threshold");
    puts("\t-c    Optional critical threshold");
    puts("\t-0    Show interfaces in DOWN state");
}

static size_t _ifNumber = 0;
static char **_ifAlias;
static size_t *_ifAlias_len;
static ifEntry64_t *_ifSpeed;
static ifEntry64_t *_ifInOctets;
static ifEntry64_t *_ifOutOctets;
static ifEntry8_t  *_ifAdminState;
static ifEntry8_t  *_ifOperState;

static struct if_status_t *info = NULL;
static struct if_status_t *curr = NULL;

size_t _li_alias_cc(struct variable_list *vars, size_t idx)
{
    if (idx >= _ifNumber)
        return idx;

    _ifAlias[idx] = calloc(vars->val_len + 1, sizeof(char));
    _ifAlias_len[idx] = vars->val_len;
    memmove(_ifAlias[idx], vars->val.string, vars->val_len);

    return ++idx;
}

size_t _li_descr_cc(struct variable_list *vars, size_t idx)
{
    if (idx >= _ifNumber)
        return idx;

    char *v_name;
    size_t v_name_len = vars->val_len;

    char *alias = _ifAlias[idx] /* _ifAlias->words[idx] */;
    size_t alias_len = _ifAlias_len[idx] /* _ifAlias->sizes[idx] */;

    if (alias_len == 0 || strcmp((char *)vars->val.string, alias) == 0) {
        pcre *re;
        const char *error;
        int erroffset;

        re = pcre_compile("^[0-9/]+$", 0, &error, &erroffset, NULL);

        if (re == NULL)
            exit_error(EXIT_UNKNOWN, "PCRE ERROR");

        int ovector[OVECCOUNT] = {0};
        int rc = pcre_exec(re, NULL, (char *)vars->val.string,
                           (int)vars->val_len, 0, 0, ovector, OVECCOUNT);

        pcre_free(re);

        if (rc == 1) {
            char *prefix = NUMBER_IF_PREFIX;
            size_t prefix_len = strlen(NUMBER_IF_PREFIX);
            v_name_len = vars->val_len + prefix_len;
            v_name = calloc(v_name_len + 1, sizeof(char));
            memcpy(v_name, prefix, prefix_len);
            memcpy(&v_name[prefix_len], vars->val.string, vars->val_len);
        } else {
            v_name_len = vars->val_len;
            v_name = calloc(v_name_len + 1, sizeof(char));
            memcpy(v_name, vars->val.string, vars->val_len);
        }
    }
    else {
        v_name_len = vars->val_len + alias_len + 3;
        v_name = calloc(v_name_len + 1, sizeof(char));

        memcpy(v_name, vars->val.string, vars->val_len);
        v_name[vars->val_len] = ' ';
        v_name[vars->val_len + 1] = '(';
        memcpy(&v_name[vars->val_len + 2], alias, alias_len);
        v_name[v_name_len - 1] = ')';
    }

    char *new_name = NULL;
    size_t new_name_len = 0;

    if (options.filter) {
        new_name_len = str_format(&new_name, v_name,
                                  options.filter,
                                  options.pattern);
        free(v_name);
        v_name = new_name;
        v_name_len = new_name_len;
    }

    if (!options.filter || new_name_len > 0) {
        add_info(&curr, vars->name[vars->name_length-1],
                 v_name, v_name_len,
                 _ifAdminState[idx],
                 _ifOperState[idx],
                 _ifSpeed[idx],
                 _ifInOctets[idx],
                 _ifOutOctets[idx]
                 );

        if (!info)
            info = curr;
    }

    free(v_name);

    return ++idx;
}

size_t _li_speed_cc(struct variable_list *vars, size_t idx)
{
    if (idx >= _ifNumber)
        return idx;

    _ifSpeed[idx] = (u_int64_t)(*vars->val.integer * IF_SPEED_1MB);

    return ++idx;
}

size_t _li_in_octets_cc(struct variable_list *vars, size_t idx)
{
    if (idx >= _ifNumber)
        return idx;

    _ifInOctets[idx] = ((*vars->val.counter64).high << 32) +
            (*vars->val.counter64).low;

    return ++idx;
}

size_t _li_out_octets_cc(struct variable_list *vars, size_t idx)
{
    if (idx >= _ifNumber)
        return idx;

    _ifOutOctets[idx] = ((*vars->val.counter64).high << 32) +
            (*vars->val.counter64).low;

    return ++idx;
}

size_t _li_adm_state_cc(struct variable_list *vars, size_t idx)
{
    if (idx >= _ifNumber)
        return idx;

    _ifAdminState[idx] = *vars->val.integer & 0xFF;

    return ++idx;
}

size_t _li_opr_state_cc(struct variable_list *vars, size_t idx)
{
    if (idx >= _ifNumber)
        return idx;

    _ifOperState[idx] = *vars->val.integer & 0xFF;

    return ++idx;
}

size_t ifNumber(void) {
    oid theOid[] = { 1, 3, 6, 1, 2, 1, 2, 1, 0 };

    struct snmp_pdu *response;
    size_t value = 0;
    int status = 0;

    status = get_pdu(theOid, OID_LENGTH(theOid), &response);

    check_response_errstat(response);

    switch (response->variables->type) {
    case ASN_INTEGER:
        value = *response->variables->val.integer & 0xFFFFFFFF;
        break;
    }

    snmp_free_pdu(response);

    return value;
}

struct if_status_t *load_snmp_info(void)
{
    _ifNumber = ifNumber();

    _ifAlias = (char **)calloc(_ifNumber, sizeof(char *));
    if (!_ifAlias)
        exit_error(EXIT_UNKNOWN, "Unable to allocate memory (ifAlias)");

    _ifAlias_len = (size_t *)malloc(_ifNumber * sizeof(size_t));
    if (!_ifAlias_len)
        exit_error(EXIT_UNKNOWN, "Unable to allocate memory (ifAlias_len)");

    _ifSpeed = (ifEntry64_t *)calloc(_ifNumber, sizeof(ifEntry64_t));
    if (!_ifSpeed)
        exit_error(EXIT_UNKNOWN, "Unable to allocate memory (ifSpeed)");

    _ifInOctets = (ifEntry64_t *)calloc(_ifNumber, sizeof(ifEntry64_t));
    if (!_ifInOctets)
        exit_error(EXIT_UNKNOWN, "Unable to allocate memory (ifInOctets)");

    _ifOutOctets = (ifEntry64_t *)calloc(_ifNumber, sizeof(ifEntry64_t));
    if (!_ifOutOctets)
        exit_error(EXIT_UNKNOWN, "Unable to allocate memory (ifOutOctets)");

    _ifAdminState = (ifEntry8_t *)calloc(_ifNumber, sizeof(ifEntry8_t));
    if (!_ifAdminState)
        exit_error(EXIT_UNKNOWN, "Unable to allocate memory (ifAdminState)");

    _ifOperState = (ifEntry8_t *)calloc(_ifNumber, sizeof(ifEntry8_t));
    if (!_ifOperState)
        exit_error(EXIT_UNKNOWN, "Unable to allocate memory (ifOperState)");


    oid oid_ifAlias[] = { 1, 3, 6, 1, 2, 1, 31, 1, 1, 1, 1 };
    iterate_vars(oid_ifAlias, OID_LENGTH(oid_ifAlias), 10,
                 _li_alias_cc, NULL);

    oid oid_ifHighSpeed[] = { 1, 3, 6, 1, 2, 1, 31, 1, 1, 1, 15 };
    iterate_vars(oid_ifHighSpeed, OID_LENGTH(oid_ifHighSpeed), 50,
                 _li_speed_cc, NULL);

    oid oid_ifHCInOctets[] = { 1, 3, 6, 1, 2, 1, 31, 1, 1, 1, 6 };
    iterate_vars(oid_ifHCInOctets, OID_LENGTH(oid_ifHCInOctets), 50,
                 _li_in_octets_cc, NULL);

    oid oid_ifHCOutOctets[] = { 1, 3, 6, 1, 2, 1, 31, 1, 1, 1, 10 };
    iterate_vars(oid_ifHCOutOctets, OID_LENGTH(oid_ifHCOutOctets), 50,
                 _li_out_octets_cc, NULL);

    oid oid_ifAdminState[] = { 1, 3, 6, 1, 2, 1, 2, 2, 1, 7 };
    iterate_vars(oid_ifAdminState, OID_LENGTH(oid_ifAdminState), 50,
                 _li_adm_state_cc, NULL);

    oid oid_ifOperState[] = { 1, 3, 6, 1, 2, 1, 2, 2, 1, 8 };
    iterate_vars(oid_ifOperState, OID_LENGTH(oid_ifOperState), 50,
                 _li_opr_state_cc, NULL);

    oid oid_ifDescr[] = { 1, 3, 6, 1, 2, 1, 2, 2, 1, 2 };
    iterate_vars(oid_ifDescr, OID_LENGTH(oid_ifDescr), 10,
                 _li_descr_cc, NULL);

    free(_ifOperState);
    free(_ifAdminState);
    free(_ifOutOctets);
    free(_ifInOctets);
    free(_ifSpeed);
    free(_ifAlias_len);

    for (size_t i = 0; i < _ifNumber; i++)
        free(_ifAlias[i]);

    free(_ifAlias);

    return info;
}

size_t str_format(char **result, const char *subject,
                  const char *pattern, char *format)
{
    pcre *re;
    const char *error;
    int erroffset;
    int rc;
    int i;
    size_t subject_len = strlen(subject);
    size_t format_len = format == NULL ? 0 : strlen(format);
    int ovector[OVECCOUNT] = {0};

    if (format_len == 0) {
        format = "$0";
        format_len = 2;
    }

    re = pcre_compile(pattern, 0, &error, &erroffset, NULL);

    if (re == NULL)
        return 0;

    rc = pcre_exec(re, NULL, subject, (int)subject_len,
                   0, 0, ovector, OVECCOUNT);

    pcre_free(re);

    if (rc <= 0 || rc > 99)
        return 0;

    size_t substr_len = 0;    // Substrings length
    size_t pat_len = 0;       // Patterns length
    size_t pat_sizes[OVECCOUNT / 2] = {0};
    char pat_vector[OVECCOUNT / 2][4];
    char *tmp;

    for (i = 0; i < rc; i++) {
        if (ovector[2*i] < 0)
            continue;

        pat_sizes[i] = 1 + (i < 10 ? 1 : 2);
        snprintf(pat_vector[i], pat_sizes[i] + 1, "$%d", i);

        if ((tmp = strstr(format, pat_vector[i])) == NULL)
            continue;

        substr_len += (size_t)(ovector[2*i+1] - ovector[2*i]);
        pat_len += pat_sizes[i];
    }

    char *_result = calloc(4096, sizeof(char));
    size_t insert_size = 0;
    size_t new_size = format_len - pat_len + substr_len;
    memcpy(_result, format, format_len);

    size_t start = 0;
    size_t end = format_len;
    for (i = 0; i < rc; i++) {
        if (ovector[2*i] < 0)
            continue;

        if (!(tmp = strstr(_result, pat_vector[i])))
            continue;

        start = (size_t)(tmp - _result) + pat_sizes[i];
        insert_size = (size_t)(ovector[2*i+1] - ovector[2*i]);

        for (size_t k = end; k >= start; --k)
            _result[k + insert_size - pat_sizes[i]] = _result[k];

        memcpy(tmp, &subject[ovector[2*i]], insert_size);
        end += insert_size - pat_sizes[i];
    }

    *result = calloc(new_size + 1, sizeof(char));
    memcpy(*result, _result, new_size);
    free(_result);

    return new_size;
}
