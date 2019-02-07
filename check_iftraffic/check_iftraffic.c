#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pcre.h>

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

    while ((opt = getopt(argc, argv, "H:C:w:c:20hf:t:p:")) != -1)
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

    if (!options.version)
        options.version = SNMP_VERSION_1;

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
    puts("\t-2    Use SNMP version 2c");
    puts("\t-w    Optional warning threshold");
    puts("\t-c    Optional critical threshold");
    puts("\t-0    Show interfaces in DOWN state");
}

struct if_status_t *load_snmp_info(void)
{
    struct snmp_pdu *response;
    struct variable_list *vars;

    int status = 0;
    int can_go_next = 1;

    oid name[MAX_OID_LEN];
    size_t name_len = MAX_OID_LEN;

    oid end_oid[MAX_OID_LEN];
    size_t end_oid_len = MAX_OID_LEN;

    struct if_status_t *info = NULL;
    struct if_status_t *curr = NULL;

    oid root[] = { 1, 3, 6, 1, 2, 1, 2, 2, 1, 2 };

    memmove(end_oid, root, sizeof(root));
    end_oid_len = OID_LENGTH(root);
    end_oid[end_oid_len-1]++;

    memmove(name, root, sizeof(root));
    name_len = OID_LENGTH(root);

#ifdef DEBUG
    printf("Filter: >%s<\nPattern: >%s<\n", options.filter, options.pattern);
#endif

    while (can_go_next) {
        status = get_pdu_next(name, name_len, &response);
        check_response_errstat(&response);

        for (vars = response->variables; vars; vars = vars->next_variable) {
            if (snmp_oid_compare(end_oid, end_oid_len,
                                 vars->name, vars->name_length) <= 0) {
                can_go_next = 0;
                continue;
            }

            oid theOid = vars->name[vars->name_length-1];

            char *v_name;
            size_t v_name_len = 0;

            char *alias = ifEntryAlias(theOid);
            size_t alias_len = strlen(alias);

            if (strcmp((char *)vars->val.string, alias) == 0) {
                v_name_len = vars->val_len;
                v_name = malloc(v_name_len * sizeof(char) + 1);

                memcpy(v_name, vars->val.string, vars->val_len);
                v_name[v_name_len] = '\0';
            }
            else {
                v_name_len = vars->val_len + alias_len + 3;
                v_name = malloc(v_name_len * sizeof(char) + 1);

                memcpy(v_name, vars->val.string, vars->val_len);
                strcat(&v_name[vars->val_len], " (");
                v_name[vars->val_len] = ' ';
                v_name[vars->val_len + 1] = '(';
                memcpy(&v_name[vars->val_len + 2], alias, alias_len);
                v_name[v_name_len - 1] = ')';
                v_name[v_name_len] = '\0';
            }

            char *new_name = NULL;
            size_t new_name_len = 0;

            if (options.filter) {
                new_name_len = str_format(&new_name, v_name,
                                          options.filter,
                                          options.pattern);
                v_name_len = new_name_len;
            }

            if (!options.filter || new_name_len > 0) {
                fill_info(&curr, vars, v_name, v_name_len);

                if (!info)
                    info = curr;
            }

            free(v_name);
            free(alias);

            memmove((char *)name, (char *)vars->name,
                    vars->name_length * sizeof(oid));
            name_len = vars->name_length;
        }

        if (response)
            snmp_free_pdu(response);
    }

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
    int ovector[OVECCOUNT] = {0};

    if (format == NULL || strlen(format) == 0)
        format = "$0";

    re = pcre_compile(pattern, 0, &error, &erroffset, NULL);

    if (re == NULL)
        return 0;

    rc = pcre_exec(re, NULL, subject, (int)subject_len,
                   0, 0, ovector, OVECCOUNT);

    if (rc <= 0) {
        pcre_free(re);
        return 0;
    }

    size_t substr_len = 0;    // Substrings length
    size_t pat_len = 0;       // Patterns length
    size_t pat_sizes[OVECCOUNT / 2] = {0};
    char pat_vector[OVECCOUNT / 2][10];
    char *tmp;

    for (i = 0; i < rc; i++) {
        if (ovector[2*i] < 0)
            continue;

        snprintf(pat_vector[i], 10, "$%d", i);
        pat_sizes[i] = strlen(pat_vector[i]);

        tmp = strstr(format, pat_vector[i]);
        if (!tmp)
            continue;

        substr_len += (size_t)(ovector[2*i+1] - ovector[2*i]);

        pat_len += pat_sizes[i];
    }

    size_t format_size = strlen(format);
    size_t new_size = format_size - pat_len + substr_len;
    char *_result = calloc(new_size + 1, sizeof(char));
    memcpy(_result, format, format_size);

    size_t start = 0;
    size_t insert_size = 0;
    size_t end = format_size;
    for (i = 0; i < rc; i++) {
        if (!pat_sizes[i])
            continue;

        tmp = strstr(_result, pat_vector[i]);
        if (!tmp)
            continue;

        start = (size_t)(tmp - _result) + pat_sizes[i];
        insert_size = (size_t)(ovector[2*i+1] - ovector[2*i]);

        for (size_t k = end; k >= start; --k)
            _result[k + insert_size - pat_sizes[i]] = _result[k];

        memcpy(tmp, &subject[ovector[2*i]], insert_size);

        end += insert_size - pat_sizes[i];
    }

    if (re)
        pcre_free(re);

    *result = _result;

    return new_size;
}