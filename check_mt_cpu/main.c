#include <stdio.h>

#include "utils.h"
#include "snmp.h"

#include "check_mt_cpu.h"

struct opt_s options;
static const oid oid_cpu_load[] = { 1, 3, 6, 1, 4, 1, 2021, 11, 10, 0 };

int main(int argc, char *argv[])
{
    msg_t msg;
    load_t load = 0;
    perf_t perf;
    code_t code = EXIT_OK;

    parse_args(argc, argv);

    init_session(options.host, options.community, SNMP_VERSION_2c);
    load = get_cpu_load(oid_cpu_load,
                        sizeof(oid_cpu_load) / sizeof (oid));
    close_session();

    fix_threshold(&options.warn, &options.crit);

    if (options.crit && load >= options.crit)
        code = EXIT_CRITICAL;
    else if (options.warn && load >= options.warn)
        code = EXIT_WARNING;

    sprintf(msg, "CPU load (1 minute average) is %d%%%s", load,
            (code == EXIT_CRITICAL ? "!!" : (code == EXIT_WARNING ? "!" : "")));

    if (options.warn == 0 && options.crit == 0)
        sprintf(perf, "'load1'=%d%%;;;;", load);
    else if (options.warn == 0)
        sprintf(perf, "'load1'=%d%%;%d;;;", load, options.warn);
    else if (options.crit == 0)
        sprintf(perf, "'load1'=%d%%;;%d;;", load, options.crit);
    else
        sprintf(perf, "'load1'=%d%%;%d;%d;;", load, options.warn, options.crit);

    exit_ok(code, msg, perf);
}
