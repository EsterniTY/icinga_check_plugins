#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pcre.h>
#include <callback.h>
#include <openssl/md5.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "snmp.h"
#include "utils.h"
#include "if_status.h"
#include "info.h"
#include "perfdata.h"

#include "check_iftraffic.h"

extern const char *__progname;
static const char *__version = "1.0";

static size_t        _ifNumber = 0;
static char        **_ifAlias;
static size_t       *_ifAlias_len;
static ifEntry64_t  *_ifSpeed;
static ifEntry64_t  *_ifInOctets;
static ifEntry64_t  *_ifOutOctets;
static ifEntry8_t   *_ifAdminState;
static ifEntry8_t   *_ifOperState;
static ifEntry64_t  *_ifInUcastPkts;
static ifEntry64_t  *_ifOutUcastPkts;

static struct if_status_t *info = NULL;
static struct if_status_t *curr = NULL;

void parse_args(int argc, char *argv[])
{
    int opt;
    options.host = NULL;
    options.community = "public";
    options.version = SNMP_VERSION_2c;
    options.warn = 0;
    options.crit = 0;
    options.cache_dir = "/tmp/mt";
    options.filter = NULL;
    options.pattern = NULL;
    options.downstate = 0;
    options.speed = 0;

    while ((opt = getopt(argc, argv, "H:C:w:c:120hf:t:p:S:")) != -1)
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
        case 'S':
            options.speed = (u_long) ((u_int) atoi(optarg)) * 1000000;
            break;
        case 'h':
            print_help();
            exit(0);
        }
    }

    if (options.host == NULL)
        exit_error(EXIT_CRITICAL, "No host defined");

    char *uid = calloc(256, sizeof(char));
    snprintf(uid, 256, "%s;%s;%s;%s;%d;%ld;%lu;%s",
             options.host, options.community,
             options.filter, options.pattern,
             options.downstate, options.version, options.speed,
             getenv("USER"));

    unsigned char digest[16];
    char *md5 = (char*)calloc(33, sizeof(char));
    MD5_CTX c;
    size_t length = strlen(uid);

    MD5_Init(&c);
    MD5_Update(&c, uid, length);
    MD5_Final(digest, &c);

    for (int n = 0; n < 16; ++n) {
        snprintf(&(md5[n*2]), 3, "%02x", (unsigned int)digest[n]);
    }

    size_t path_len = strlen(options.cache_dir) + strlen(options.host)
            + 32 // strlen(md5)
            + 6 // strlen(of_slashes_dots_and_extension)
            + 1; // NULL byte

    options.cache_path = calloc(path_len, sizeof(char));
    sprintf(options.cache_path, "%s/%s-%s.dat",
            options.cache_dir, options.host, md5);

    free(uid);
    free(md5);
}

void print_help()
{
    printf("Check interface utilisation. ");
    printf("Version %s.\n\n", __version);
    printf("Usage: %s -H <host_address> [-C <community>] [-1|-2]\n", __progname);
    puts("\t[-w <warning>] [-c <critical>] [-S <speed>]");
    puts("\t[-0] [-f <filter>] [-p <pattern>] [-t <dir_path>]");
    puts("Options:");
    puts("\t-H    Host to check");
    puts("\t-C    SNMP community name ('public' is used if ommited)");
    puts("\t-1    Use SNMP version 1");
    puts("\t-2    Use SNMP version 2c (default)");
    puts("\t-w    Optional warning threshold (in percent)");
    puts("\t-c    Optional critical threshold (in percent)");
    puts("\t-S    Manualy specify interface speed (in megabytes)");
    puts("\t-0    Show interfaces in DOWN state");
    puts("\t-f    Interfaces filter (perl compotable regular expression)");
    puts("\t-p    Interfaces pattern (use $1, $2, etc to include appropriate");
    puts("\t      capturing group string, defined in -f option)");
    puts("\t-t    Cache directory (/tmp/mt if ommited)");
    puts("\t-h    Show this help");
}

void add_msg(const struct if_status_t *item,
             char ***stack, const size_t count,
             const double in_p, const double out_p,
             const bytes_t in, const bytes_t out
             )
{
    char **_stack = *stack;

    if (_stack == NULL)
        _stack = (char **)malloc(sizeof(char *));
    else
        _stack = (char **)realloc(_stack, (1 + count) * sizeof(char *));


    char buf[MESSAGE_BUFER_SIZE] = {0};
    if (item->alias_len == 0 || strcmp(item->name, item->alias) == 0)
        snprintf(buf, 160, IF_USAGE,
                item->name,
                in_p,
                (in_p>=options.crit ? "!!" : (in_p>=options.warn ? "!" : "")),
                in,
                out_p,
                (out_p>=options.crit ? "!!" : (out_p>=options.warn ? "!" : "")),
                out);
    else
        snprintf(buf, 160, IF_USAGE_ALIAS,
                item->name, item->alias,
                in_p,
                (in_p>=options.crit ? "!!" : (in_p>=options.warn ? "!" : "")),
                in,
                out_p,
                (out_p>=options.crit ? "!!" : (out_p>=options.warn ? "!" : "")),
                out);

    size_t msg_len = strlen(buf);
    _stack[count] = (char *)calloc(msg_len+1, sizeof(char));
    memmove(_stack[count], buf, msg_len);

    *stack = _stack;
}

size_t _li_alias_cc(struct variable_list *vars, size_t idx)
{
    CHECK_IDX;

    _ifAlias[idx] = calloc(vars->val_len + 1, sizeof(char));
    _ifAlias_len[idx] = vars->val_len;
    memmove(_ifAlias[idx], vars->val.string, vars->val_len);

    return ++idx;
}

size_t _li_descr_cc(struct variable_list *vars, size_t idx)
{
    CHECK_IDX;

    if (options.downstate == 0 &&
            (_ifOperState[idx] != IF_OPER_UP ||
             _ifAdminState[idx] != IF_ADMIN_UP))
            return ++idx;

    char *v_name;
    size_t v_name_len = vars->val_len;

    char *alias = _ifAlias[idx];
    size_t alias_len = _ifAlias_len[idx];

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
                 _ifAlias[idx], _ifAlias_len[idx],
                 _ifAdminState[idx],
                 _ifOperState[idx],
                 _ifSpeed[idx],
                 _ifInOctets[idx],
                 _ifOutOctets[idx],
                 _ifInUcastPkts[idx],
                 _ifOutUcastPkts[idx]
                 );

        if (!info)
            info = curr;
    }

    free(v_name);

    return ++idx;
}

size_t _li_speed_cc(struct variable_list *vars, size_t idx)
{
    CHECK_IDX;

    _ifSpeed[idx] = (u_int64_t)(*vars->val.integer * IF_SPEED_1MB);

    return ++idx;
}

size_t _li_in_octets_cc(struct variable_list *vars, size_t idx)
{
    CHECK_IDX;

    _ifInOctets[idx] = GET_COUNTER64();

    return ++idx;
}

size_t _li_in_ucast_cc(struct variable_list *vars, size_t idx)
{
    CHECK_IDX;

    _ifInUcastPkts[idx] = GET_COUNTER64();

    return ++idx;
}

size_t _li_out_ucast_cc(struct variable_list *vars, size_t idx)
{
    CHECK_IDX;

    _ifOutUcastPkts[idx] = GET_COUNTER64();

    return ++idx;
}

size_t _li_out_octets_cc(struct variable_list *vars, size_t idx)
{
    CHECK_IDX;

    _ifOutOctets[idx] = GET_COUNTER64();

    return ++idx;
}

size_t _li_adm_state_cc(struct variable_list *vars, size_t idx)
{
    CHECK_IDX;

    _ifAdminState[idx] = *vars->val.integer & 0xFF;

    return ++idx;
}

size_t _li_opr_state_cc(struct variable_list *vars, size_t idx)
{
    CHECK_IDX;

    _ifOperState[idx] = *vars->val.integer & 0xFF;

    return ++idx;
}

size_t ifNumber(void) {
    oid              theOid[] = {1,3,6,1,2,1,2,1,0};
    struct snmp_pdu *response;
    size_t           value    = 0;

    get_pdu(theOid, OID_LENGTH(theOid), &response);
    check_response_errstat(response);

    if (response->variables->type == ASN_INTEGER)
        value = *response->variables->val.integer & 0xFFFFFFFF;

    snmp_free_pdu(response);

    return value;
}

struct if_status_t *load_snmp_info(void)
{
    _ifNumber = ifNumber();

    IF_ALLOC(_ifAlias, char *);
    IF_ALLOC(_ifAlias_len, size_t);
    IF_ALLOC_64(_ifSpeed);
    IF_ALLOC_64(_ifInOctets);
    IF_ALLOC_64(_ifOutOctets);
    IF_ALLOC_64(_ifInUcastPkts);
    IF_ALLOC_64(_ifOutUcastPkts);
    IF_ALLOC_8(_ifAdminState);
    IF_ALLOC_8(_ifOperState);

    iterate_vars((oid[]){1,3,6,1,2,1,31,1,1,1,18}, 11, 10, _li_alias_cc);
    iterate_vars((oid[]){1,3,6,1,2,1,31,1,1,1,15}, 11, 50, _li_speed_cc);
    iterate_vars((oid[]){1,3,6,1,2,1,31,1,1,1,6 }, 11, 50, _li_in_octets_cc);
    iterate_vars((oid[]){1,3,6,1,2,1,31,1,1,1,10}, 11, 50, _li_out_octets_cc);
    iterate_vars((oid[]){1,3,6,1,2,1,31,1,1,1,7 }, 11, 50, _li_in_ucast_cc);
    iterate_vars((oid[]){1,3,6,1,2,1,31,1,1,1,11}, 11, 50, _li_out_ucast_cc);
    iterate_vars((oid[]){1,3,6,1,2,1,2 ,2,1,7   }, 10, 50, _li_adm_state_cc);
    iterate_vars((oid[]){1,3,6,1,2,1,2 ,2,1,8   }, 10, 50, _li_opr_state_cc);
    iterate_vars((oid[]){1,3,6,1,2,1,31,1,1,1,1 }, 11, 10, _li_descr_cc);

    free(_ifOperState);
    free(_ifAdminState);
    free(_ifOutOctets);
    free(_ifInOctets);
    free(_ifInUcastPkts);
    free(_ifOutUcastPkts);
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

u_int get_host_uptime()
{
    oid              theOid[]  = {1,3,6,1,2,1,1,3,0};
    struct snmp_pdu *response;
    u_int            value     = 0;

    get_pdu(theOid, OID_LENGTH(theOid), &response);
    check_response_errstat(response);

    if (response->variables->type == ASN_TIMETICKS)
        value = (u_int) *response->variables->val.integer;

    snmp_free_pdu(response);

    return value;
}
