#include <stdio.h>

#include "utils.h"
#include "snmp.h"

#include "check_mt_temp.h"

struct opt_s options;
static const oid oid_hw_temp[] = { 1, 3, 6, 1, 4, 1, 14988, 1, 1, 3, 10, 0 };
static const oid oid_cpu_temp[] = { 1, 3, 6, 1, 4, 1, 14988, 1, 1, 3, 11, 0 };

int main(int argc, char *argv[])
{
    temp_t cpu_temp = 0;
    temp_t hw_temp = 0;

    parse_args(argc, argv);

    init_session(options.host, options.community, SNMP_VERSION_2c);

    if ((options.mode & MODE_CPU) == MODE_CPU)
        cpu_temp = get_temp(oid_cpu_temp,
                            sizeof(oid_cpu_temp) / sizeof (oid));

    if ((options.mode & MODE_HW) == MODE_HW)
        hw_temp = get_temp(oid_hw_temp,
                           sizeof(oid_hw_temp) / sizeof (oid));

    close_session();

    if (cpu_temp && !options.cpu_warn)
        options.cpu_warn = options.hw_warn;
    if (cpu_temp && !options.cpu_crit)
        options.cpu_crit = options.hw_crit;

    fix_threshold(&options.hw_warn, &options.hw_crit);
    fix_threshold(&options.cpu_warn, &options.cpu_crit);

    code_t cpu_code = EXIT_OK;
    code_t hw_code = EXIT_OK;

    msg_t cpu_msg;
    msg_t hw_msg;
    perf_t cpu_perf;
    perf_t hw_perf;

    if (cpu_temp)
        cpu_code = check_temp(cpu_temp, options.cpu_warn, options.cpu_crit,
                              "CPU", "cpu_temp",
                              &cpu_msg, &cpu_perf);

    if (hw_temp)
        hw_code = check_temp(hw_temp, options.hw_warn, options.hw_crit,
                             "HW", "hw_temp",
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
