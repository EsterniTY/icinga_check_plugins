#include <stdio.h>
#include <locale.h>

#include "snmp.h"

#include "if_status.h"
#include "info.h"
#include "file.h"
#include "perfdata.h"

#ifdef DEBUG
#include "debug.h"
#endif

#include "check_iftraffic.h"

struct opt_s options;

int main(int argc, char *argv[])
{
    parse_args(argc, argv);

#ifdef DEBUG
    setlocale(LC_NUMERIC, "");
#endif

    struct if_status_t *old_info = NULL;
    struct if_status_t *new_info = NULL;

    init_session(options.host, options.community, options.version);
    new_info = load_snmp_info();
    close_session();

    old_info = read_info();
    write_info(new_info);

    if (old_info == NULL) {
        if (new_info)
            free_info(new_info);

        exit_error(EXIT_UNKNOWN, "Collecting data");
    }

    struct if_status_t *new = NULL;
    struct if_status_t *old = NULL;
    new = new_info;

    struct perfdata *pf = NULL;
    struct perfdata *pf_curr = pf;

#ifdef DEBUG
    spacer("New Info");
    print_info_table(new_info);
    spacer("Old Info");
    print_info_table(old_info);
    spacer("Delta");
    printf(format1, "oid", "ifName", "Speed?", "deltaTime (s)",
           "inDelta (b)", "outDelta (b)", "in bps", "out bps");
#endif

    while (new) {
        bytes_t inDelta = 0;
        bytes_t outDelta = 0;
        mtime_t timeDelta = 1;

        if (old_info) {
            old = old_info;
            while (old) {
                if (strcmp(new->name, old->name) == 0) {
                    inDelta = new->inOctets - old->inOctets;
                    outDelta = new->outOctets - old->outOctets;
                    timeDelta = (new->microtime - old->microtime) / 1000;
                    break;
                }

                old = old->next;
            }
        }

        char pf_name[40];
        int pf_len;

        bytes_t in_bps;
        bytes_t out_bps;
        bytes_t warn;
        bytes_t crit;

        double out_percent;
        double in_percent;

        in_bps = timeDelta  ? inDelta / timeDelta : 0;
        out_bps = timeDelta ? outDelta / timeDelta : 0;
        warn = options.warn * (new->speed / 100);
        crit = options.crit * (new->speed / 100);
        out_percent = (double)out_bps * 100 / new->speed;
        in_percent = (double)in_bps * 100 / new->speed;

        pf_len = snprintf(pf_name, 40, "%s_traffic_in", new->name);
        perfdata_add_bytes(&pf_curr, pf_name, (size_t) pf_len,
                           in_bps, warn, crit, 0, new->speed);

        if (!pf)
            pf = pf_curr;

        pf_len = snprintf(pf_name, 40, "%s_traffic_out", new->name);
        perfdata_add_bytes(&pf_curr, pf_name, (size_t) pf_len,
                           out_bps, warn, crit, 0, new->speed);

        pf_len = snprintf(pf_name, 40, "%s_usage_out", new->name);
        perfdata_add_percent(&pf_curr, pf_name, (size_t) pf_len,
                             out_percent, options.warn, options.crit,
                             0, 100);

        pf_len = snprintf(pf_name, 40, "%s_usage_in", new->name);
        perfdata_add_percent(&pf_curr, pf_name, (size_t) pf_len,
                             in_percent, options.warn, options.crit,
                             0, 100);

#ifdef DEBUG
        print_delta_row(new->id, new->name, new->speed,
                        timeDelta, inDelta, outDelta,
                        in_bps, out_bps);
#endif

        new = new->next;
    }

#ifdef DEBUG
    spacer("PerfData");
    spacer("Debug");
    printf("PDU Requests: %lu\n", _get_pdu_requests());
#else
    perfdata_print(pf);
#endif

    perfdata_free(pf);

    if (old_info)
        free_info(old_info);

    if (new_info)
        free_info(new_info);
}
