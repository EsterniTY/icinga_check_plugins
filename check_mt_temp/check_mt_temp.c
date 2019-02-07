#include <stdio.h>

#include "utils.h"
#include "snmp.h"
#include "check_mt_temp.h"

extern const char *__progname;
static const char *__version = "1.0";

temp_t get_temp(const oid *oid, size_t oid_length)
{
    struct snmp_pdu *response;
    struct variable_list *vars;
    temp_t temp = 0;

    get_pdu(oid, oid_length, &response);
    check_response_errstat(response);

    vars = response->variables;

    if (vars->type == ASN_INTEGER)
        temp = (temp_t)(*vars->val.integer / 10) & 255;

    snmp_free_pdu(response);

    return temp;
}

void parse_args(int argc, char *argv[])
{
    int opt;

    options.mode = MODE_BOTH;

    while ((opt = getopt(argc, argv, "H:C:w:c:x:y:m:h")) != -1)
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
            options.hw_warn = (temp_t)atoi(optarg);
            break;
        case 'c':
            options.hw_crit = (temp_t)atoi(optarg);
            break;
        case 'x':
            options.cpu_warn = (temp_t)atoi(optarg);
            break;
        case 'y':
            options.cpu_crit = (temp_t)atoi(optarg);
            break;
        case 'm':
            if (strcmp(optarg, "cpu") == 0)
                options.mode = MODE_CPU;
            else if (strcmp(optarg, "hw") == 0)
                options.mode = MODE_HW;
            else if (strcmp(optarg, "ok") == 0)
                options.mode = MODE_OK;
            else
                options.mode = MODE_BOTH;
            break;
        case 'h':
            print_help();
            exit(0);
        }
    }

    if (options.host == NULL)
        exit_error(EXIT_CRITICAL, "No host defined");

    if (options.mode == MODE_OK)
        exit_error(EXIT_OK, "OK mode selected. No checks performed");

    if (options.community == NULL)
        options.community = "public";
}

void print_help(void)
{
    printf("Check RouterOS hardware temperature. ");
    printf("Version %s.\n\n", __version);
    printf("Usage: %s -H <host_address> [-C <community>]\n", __progname);
    puts("\t[-m <both|cpu|hw|ok>]");
    puts("\t[-w <hw warning>] [-c <hw critical>]");
    puts("\t[-x <cpu warning>] [-y <cpu critical>]\n");
    puts("Options:");
    puts("\t-H    Host to check");
    puts("\t-C    SNMP community name ('public' is used if omitted)");
    puts("\t-m    Check mode (default is 'both'):");
    puts("\t\tcpu  - for CPU check only");
    puts("\t\thw   - for Hardware check only");
    puts("\t\tboth - for CPU and hardware temperature checks");
    puts("\t\tok   - will return OK status on unsupported devices");
    puts("\t-w    Optional warning threshold");
    puts("\t-c    Optional critical threshold");
    puts("\t-x    CPU critical threshold");
    puts("\t-y    CPU critical threshold");
}

code_t check_temp(const temp_t temp,
                  const temp_t warn,
                  const temp_t crit,
                  const char *msg_str,
                  const char *perf_str,
                  msg_t *msg,
                  perf_t *perf)
{
    code_t code = EXIT_OK;

    if (crit && temp >= crit)
        code = EXIT_CRITICAL;
    else if (warn && temp >= warn)
        code = EXIT_WARNING;

    sprintf(*msg, "%s temperature is %dC%s", msg_str, temp,
            (code == EXIT_CRITICAL ? "!!" : (code == EXIT_WARNING ? "!" : "")));
    format_perfdata(*perf, perf_str, temp, warn, crit);

    return code;
}
