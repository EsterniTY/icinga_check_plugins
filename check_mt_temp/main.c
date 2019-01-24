#include <stdio.h>

#include "utils.h"
#include "snmp.h"

#include "check_mt_temp.h"

static const char *oid_hw_temp = ".1.3.6.1.4.1.14988.1.1.3.10.0";
static const char *oid_cpu_temp = ".1.3.6.1.4.1.14988.1.1.3.11.0";
static u_int mode = MODE_BOTH;

int main(int argc, char *argv[])
{
    int opt;
    char *host = NULL;
    char *community = NULL;
    temp_t hw_warn = 0;
    temp_t hw_crit = 0;
    temp_t cpu_warn = 0;
    temp_t cpu_crit = 0;
    temp_t hw_temp = 0;
    temp_t cpu_temp = 0;

    while ((opt = getopt(argc, argv, "H:C:w:c:x:y:m:h")) != -1)
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
            hw_warn = (temp_t)atoi(optarg);
            break;
        case 'c':
            hw_crit = (temp_t)atoi(optarg);
            break;
        case 'x':
            cpu_warn = (temp_t)atoi(optarg);
            break;
        case 'y':
            cpu_crit = (temp_t)atoi(optarg);
            break;
        case 'm':
            if (strcmp(optarg, "cpu") == 0)
                mode = MODE_CPU;
            else if (strcmp(optarg, "hw") == 0)
                mode = MODE_HW;
            else if (strcmp(optarg, "ok") == 0)
                mode = MODE_OK;
            else
                mode = MODE_BOTH;
            break;
        case 'h':
            print_help();
            exit(0);
        }
    }

    if (host == NULL)
        exit_error(EXIT_CRITICAL, "No host defined");

    if (mode == MODE_OK)
        exit_error(EXIT_OK, "OK mode selected. No checks performed");

    if (community == NULL)
        community = "public";

    init_session(host, community);

    if ((mode & MODE_CPU) == MODE_CPU)
        cpu_temp = get_hw_temperature(oid_cpu_temp);

    if ((mode & MODE_HW) == MODE_HW)
        hw_temp = get_hw_temperature(oid_hw_temp);

    close_session();

    if (cpu_temp && !cpu_warn)
        cpu_warn = hw_warn;
    if (cpu_temp && !cpu_crit)
        cpu_crit = hw_crit;

    fix_threshold(&hw_warn, &hw_crit);
    fix_threshold(&cpu_warn, &cpu_crit);

    code_t cpu_code = EXIT_OK;
    code_t hw_code = EXIT_OK;

    msg_t cpu_msg;
    msg_t hw_msg;
    perf_t cpu_perf;
    perf_t hw_perf;

    if (cpu_temp)
        cpu_code = check_temp(cpu_temp, cpu_warn, cpu_crit, "CPU", "cpu_temp",
                              &cpu_msg, &cpu_perf);

    if (hw_temp)
        hw_code = check_temp(hw_temp, hw_warn, hw_crit, "HW", "hw_temp",
                             &hw_msg, &hw_perf);

    if (cpu_temp && hw_temp)
    {
        char msg[80];
        char perfdata[100];

        if (hw_code > EXIT_OK)
            sprintf(msg, "%s, %s", hw_msg, cpu_msg);
        else
            sprintf(msg, "%s, %s", cpu_msg, hw_msg);

        sprintf(perfdata, "%s %s", cpu_perf, hw_perf);

        if (hw_code > cpu_code)
            exit_ok(hw_code, msg, perfdata);
        else
            exit_ok(cpu_code, msg, perfdata);
    }
    else if (cpu_temp)
        exit_ok(cpu_code, cpu_msg, cpu_perf);
    else if (hw_temp)
        exit_ok(hw_code, hw_msg, hw_perf);
    else
        exit_error(EXIT_OK, "No measurements available");

    exit_error(EXIT_UNKNOWN, "Some strange error can I see");
}
