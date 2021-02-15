#include <stdio.h>
#include <unistd.h>

#include "utils.h"
#include "snmp.h"

#include "check_mt_cpu.h"

extern const char *__progname;
static const char *__version = "1.0";

void parse_args(int argc, char *argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, "H:C:w:c:h")) != -1)
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
            options.warn = (load_t)atoi(optarg);
            break;
        case 'c':
            options.crit = (load_t)atoi(optarg);
            break;
        case 'h':
            print_help();
            exit(0);
        }
    }

    if (options.host == NULL)
        exit_error(EXIT_CRITICAL, "No host defined");

    if (options.community == NULL)
        options.community = "public";
}

void print_help(void)
{
    printf("Check RouterOS CPU load. ");
    printf("Version %s.\n\n", __version);
    printf("Usage: %s -H <host_address> [-C <community>]\n", __progname);
    puts("\t[-w <warning>] [-c <critical>]");
    puts("Options:");
    puts("\t-H    Host to check");
    puts("\t-C    SNMP community name ('public' is used if ommited)");
    puts("\t-w    Optional warning threshold");
    puts("\t-c    Optional critical threshold");
}

load_t get_cpu_load(const oid *oid, const size_t oid_length)
{
    struct snmp_pdu *response;
    struct variable_list *vars;
    load_t temp = 0;

    get_pdu(oid, oid_length, &response);
    check_response_errstat(response);

    vars = response->variables;

    if (vars->type == ASN_INTEGER)
        temp = (load_t)*vars->val.integer;

    snmp_free_pdu(response);

    return temp;
}
