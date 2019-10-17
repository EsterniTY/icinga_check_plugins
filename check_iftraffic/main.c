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
struct host_settings_s host_settings;

int main(int argc, char *argv[])
{
    parse_args(argc, argv);

    host_settings.has_ifSpeed64 = 1;

#ifdef DEBUG
    printf("Host: >%s<\n", options.host);
    printf("Filter: >%s<\nPattern: >%s<\n", options.filter, options.pattern);
#endif

    // Needed for pretty messages
    setlocale(LC_NUMERIC, "");

    struct if_status_t *old_info = NULL;
    struct if_status_t *new_info = NULL;

    options.version = try_session(options.host, options.community,
                                  options.version);

    host_settings.uptime = get_host_uptime();

#ifdef DEBUG
    printf("Uptime: >%u<\n", host_settings.uptime);
#endif

    new_info = load_snmp_info();
    close_session();

    old_info = read_info();

    if (old_info == NULL) {
        write_info(new_info);
        free_info(new_info);
        free(options.cache_path);

        exit_error(EXIT_UNKNOWN, "Collecting data");
    }

    if ((old_info->microtime / 1000) == (new_info->microtime / 1000)) {
        free_info(old_info);
        free_info(new_info);
        free(options.cache_path);

        exit_error(EXIT_UNKNOWN, "No time delta since last run. "
                   "Wait at least one second");
    }

    write_info(new_info);

#ifdef DEBUG
    spacer("New Info");
    print_info_table(new_info);
    spacer("Old Info");
    print_info_table(old_info);
    spacer("Delta");
    printf(format1, "oid", "ifName", "Speed", "deltaTime (s)",
           "inDelta (B)", "outDelta (B)", "in bps", "out bps",
           "in pps", "out pps", "in Err", "out Err");
#endif

    struct if_status_t *new = NULL;
    struct if_status_t *old = NULL;
    new = new_info;

    struct perfdata *pf = NULL;
    struct perfdata *pf_curr = pf;

    char **msg_w = NULL;
    char **msg_c = NULL;
    size_t msg_w_count = 0;
    size_t msg_c_count = 0;

    while (new) {
        bytes_t inDelta = 0;
        bytes_t outDelta = 0;
        bytes_t inPpsDelta = 0;
        bytes_t outPpsDelta = 0;
        bytes_t inErrDelta = 0;
        bytes_t outErrDelta = 0;
        mtime_t timeDelta = 1;

        if (old_info) {
            old = old_info;
            while (old) {
                if (old->id == new->id) {
                    inDelta = octet_delta(old->inOctets, new->inOctets) * 8;
                    outDelta = octet_delta(old->outOctets, new->outOctets) * 8;
                    inPpsDelta = octet_delta(old->inUcastPkts, new->inUcastPkts);
                    outPpsDelta = octet_delta(old->outUcastPkts, new->outUcastPkts);
                    inErrDelta = octet_delta(old->inErrors, new->inErrors);
                    outErrDelta = octet_delta(old->outErrors, new->outErrors);
                    timeDelta = (new->microtime - old->microtime) / 1000;
                    break;
                }

                old = old->next;
            }
        }

        char pf_name[40];
        int pf_len;

        bytes_t speed    = options.speed ? options.speed : new->speed;
        bytes_t in_bps   = time_delta(inDelta);
        bytes_t out_bps  = time_delta(outDelta);
        bytes_t in_pps   = time_delta(inPpsDelta);
        bytes_t out_pps  = time_delta(outPpsDelta);
        bytes_t in_err   = time_delta(inErrDelta);
        bytes_t out_err  = time_delta(outErrDelta);
        bytes_t warn     = options.warn * (speed / 100);
        bytes_t crit     = options.crit * (speed / 100);

        double out_percent;
        double in_percent;

        if (speed == 0) {
            out_percent = 0;
            in_percent = 0;
        } else {
            out_percent = (double)out_bps * 100 / speed;
            in_percent = (double)in_bps * 100 / speed;
        }

        pf_len = snprintf(pf_name, 40, "%s_traffic_in", new->name);
        perfdata_add_bytes(&pf_curr, pf_name, (size_t) pf_len,
                           in_bps, warn, crit, 0, speed);

        if (!pf)
            pf = pf_curr;

        pf_len = snprintf(pf_name, 40, "%s_traffic_out", new->name);
        perfdata_add_bytes(&pf_curr, pf_name, (size_t) pf_len,
                           out_bps, warn, crit, 0, speed);

        pf_len = snprintf(pf_name, 40, "%s_usage_out", new->name);
        perfdata_add_percent(&pf_curr, pf_name, (size_t) pf_len,
                             out_percent, options.warn, options.crit,
                             0, 100);

        pf_len = snprintf(pf_name, 40, "%s_usage_in", new->name);
        perfdata_add_percent(&pf_curr, pf_name, (size_t) pf_len,
                             in_percent, options.warn, options.crit,
                             0, 100);

        pf_len = snprintf(pf_name, 40, "%s_packets_in", new->name);
        perfdata_add_normal(&pf_curr, pf_name, (size_t) pf_len,
                           in_pps, 0, 0, 0, 0);

        pf_len = snprintf(pf_name, 40, "%s_packets_out", new->name);
        perfdata_add_normal(&pf_curr, pf_name, (size_t) pf_len,
                           out_pps, 0, 0, 0, 0);

        pf_len = snprintf(pf_name, 40, "%s_errors_in", new->name);
        perfdata_add_normal(&pf_curr, pf_name, (size_t) pf_len,
                           in_err, 0, 0, 0, 0);

        pf_len = snprintf(pf_name, 40, "%s_errors_out", new->name);
        perfdata_add_normal(&pf_curr, pf_name, (size_t) pf_len,
                           out_err, 0, 0, 0, 0);

        if (options.crit > 0 &&
                (out_percent >= options.crit
                 || in_percent >= options.crit)) {
            add_msg(new, &msg_c, msg_c_count++,
                    in_percent, out_percent,
                    in_bps, out_bps);
        } else if (options.warn > 0 &&
                   (out_percent >= options.warn
                    || in_percent >= options.warn)) {
            add_msg(new, &msg_w, msg_w_count++,
                    in_percent, out_percent,
                    in_bps, out_bps);
        }

#ifdef DEBUG
        print_delta_row(new->id, new->name, speed,
                        timeDelta, inDelta, outDelta,
                        in_bps, out_bps, in_pps, out_pps,
                        in_err, out_err);
#endif

        new = new->next;
    }

    code_t exit_code = EXIT_OK;

#ifndef DEBUG
    if (msg_c_count > 0) {
        printf("CRITICAL - ");
        exit_code = EXIT_CRITICAL;
    } else if (msg_w_count) {
        printf("WARNING - ");
        exit_code = EXIT_WARNING;
    } else
        printf("OK - no problems");
#else
    if (msg_c_count > 0)
        exit_code = EXIT_CRITICAL;
    else if (msg_w_count)
        exit_code = EXIT_WARNING;
#endif

    if (msg_c_count > 0) {
#ifdef DEBUG
        spacer("Critical Messages");
        for (size_t i = 0; i < msg_c_count; i++)
            printf("[%lu] %s\n", strlen(msg_c[i]), msg_c[i]);
#else
        for (size_t i = 0; i < msg_c_count; i++) {
            if (i == msg_c_count - 1 && msg_w_count == 0)
                printf("%s", msg_c[i]);
            else
                puts(msg_c[i]);
        }
#endif
    }

    if (msg_w_count > 0) {
#ifdef DEBUG
        spacer("Warning Messages");
        for (size_t i = 0; i < msg_w_count; i++)
            printf("[%lu] %s\n", strlen(msg_w[i]), msg_w[i]);
#else
        for (size_t i = 0; i < msg_w_count; i++) {
            if (i == msg_w_count - 1)
                printf("%s", msg_w[i]);
            else
                puts(msg_w[i]);
        }
#endif
    }

#ifdef DEBUG
    spacer("PerfData");
#endif

    perfdata_print(pf);
    perfdata_free(pf);

    while (msg_c_count > 0)
        free(msg_c[--msg_c_count]);
    if (msg_c)
        free(msg_c);

    while (msg_w_count > 0)
        free(msg_w[--msg_w_count]);
    if (msg_w)
        free(msg_w);

    free_info(old_info);
    free_info(new_info);

    free(options.cache_path);

    return exit_code;
}
