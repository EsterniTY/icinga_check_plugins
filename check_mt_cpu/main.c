#include <stdio.h>

#include "utils.h"
#include "snmp.h"

#include "check_mt_cpu.h"

typedef char msg_t[40];
typedef char perf_t[50];

static const oid oid_cpu_load[] = { 1, 3, 6, 1, 4, 1, 2021, 11, 10, 0 };

int main(int argc, char *argv[])
{
    int opt;
    char *host = NULL;
    char *community = NULL;
    load_t warn = 0;
    load_t crit = 0;
    load_t load = 0;

    msg_t msg;
    perf_t perf;
    code_t code = EXIT_OK;

    while ((opt = getopt(argc, argv, "H:C:w:c:h")) != -1)
    {
        switch (opt)
        {
        case 'H':
            host = optarg;
            break;
        case 'C':
            community = optarg;
            break;
        case 'w':
            warn = (load_t)atoi(optarg);
            break;
        case 'c':
            crit = (load_t)atoi(optarg);
            break;
        case 'h':
            print_help();
            exit(0);
        }
    }

    if (host == NULL)
        exit_error(EXIT_CRITICAL, "No host defined");

    if (community == NULL)
        community = "public";

    init_session(host, community);
    load = get_cpu_load(oid_cpu_load,
                        sizeof(oid_cpu_load) / sizeof (oid));
    close_session();

    fix_threshold(&warn, &crit);

    if (crit && load >= crit)
        code = EXIT_CRITICAL;
    else if (warn && load >= warn)
        code = EXIT_WARNING;

    sprintf(msg, "CPU load (1 minute average) is %d%%%s", load,
            (code == EXIT_CRITICAL ? "!!" : (code == EXIT_WARNING ? "!" : "")));

    if (warn == 0 && crit == 0)
        sprintf(perf, "'load1'=%d%%;;;;", load);
    else if (warn == 0)
        sprintf(perf, "'load1'=%d%%;%d;;;", load, warn);
    else if (crit == 0)
        sprintf(perf, "'load1'=%d%%;;%d;;", load, crit);
    else
        sprintf(perf, "'load1'=%d%%;%d;%d;;", load, warn, crit);

    exit_ok(code, msg, perf);
}
