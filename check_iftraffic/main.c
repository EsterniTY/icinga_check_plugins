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
    host_settings.vendor = get_vendor();

#ifdef DEBUG
    printf("Uptime: >%u<\n", host_settings.uptime);
    printf("Vendor: >%u<\n", host_settings.vendor);
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
    print_delta_header();
#endif

    struct perfdata     *pf          = NULL;
    struct perfdata     *pf_curr     = pf;
    struct delta_t      *delta       = init_delta();
    char               **msg_w       = NULL;
    char               **msg_c       = NULL;
    size_t               msg_w_count = 0;
    size_t               msg_c_count = 0;

    for (struct if_status_t *new = new_info; new != NULL; new = new->next) {
        for (struct if_status_t *old = old_info; old != NULL; old = old->next) {
            if (old->id == new->id) {
                set_delta(delta, old, new);
                break;
            }
        }

        char pf_name[40];
        int pf_len;

        bytes_t warn  = options.warn * (new->speed / 100);
        bytes_t crit  = options.crit * (new->speed / 100);

        pf_len = snprintf(pf_name, 40, "%s_traffic_in", new->name);
        perfdata_add_bytes(&pf_curr, pf_name, (size_t) pf_len,
                           delta->bytes->in, warn, crit, 0, new->speed);

        if (!pf)
            pf = pf_curr;

        pf_len = snprintf(pf_name, 40, "%s_traffic_out", new->name);
        perfdata_add_bytes(&pf_curr, pf_name, (size_t) pf_len,
                           delta->bytes->out, warn, crit, 0, new->speed);

        pf_len = snprintf(pf_name, 40, "%s_usage_out", new->name);
        perfdata_add_percent(&pf_curr, pf_name, (size_t) pf_len,
                             delta->percent->out, options.warn,
                             options.crit, 0, 100);

        pf_len = snprintf(pf_name, 40, "%s_usage_in", new->name);
        perfdata_add_percent(&pf_curr, pf_name, (size_t) pf_len,
                             delta->percent->in,
                             options.warn, options.crit, 0, 100);

        pf_len = snprintf(pf_name, 40, "%s_packets_in", new->name);
        perfdata_add_normal(&pf_curr, pf_name, (size_t) pf_len,
                           delta->packets->in, 0, 0, 0, 0);

        pf_len = snprintf(pf_name, 40, "%s_packets_out", new->name);
        perfdata_add_normal(&pf_curr, pf_name, (size_t) pf_len,
                           delta->packets->out, 0, 0, 0, 0);

        pf_len = snprintf(pf_name, 40, "%s_mcast_in", new->name);
        perfdata_add_normal(&pf_curr, pf_name, (size_t) pf_len,
                           delta->mcast->in, 0, 0, 0, 0);

        pf_len = snprintf(pf_name, 40, "%s_mcast_out", new->name);
        perfdata_add_normal(&pf_curr, pf_name, (size_t) pf_len,
                           delta->mcast->out, 0, 0, 0, 0);

        pf_len = snprintf(pf_name, 40, "%s_bcast_in", new->name);
        perfdata_add_normal(&pf_curr, pf_name, (size_t) pf_len,
                           delta->bcast->in, 0, 0, 0, 0);

        pf_len = snprintf(pf_name, 40, "%s_bcast_out", new->name);
        perfdata_add_normal(&pf_curr, pf_name, (size_t) pf_len,
                           delta->bcast->out, 0, 0, 0, 0);

        pf_len = snprintf(pf_name, 40, "%s_errors_in", new->name);
        perfdata_add_normal(&pf_curr, pf_name, (size_t) pf_len,
                           delta->errors->in, 0, 0, 0, 0);

        pf_len = snprintf(pf_name, 40, "%s_errors_out", new->name);
        perfdata_add_normal(&pf_curr, pf_name, (size_t) pf_len,
                           delta->errors->out, 0, 0, 0, 0);

        if (check_percent(options.crit, delta)) {
            add_msg(new, &msg_c, msg_c_count++,
                    delta->percent->in, delta->percent->out,
                    delta->bytes->in, delta->bytes->out);
        }
        else if (check_percent(options.warn, delta)) {
            add_msg(new, &msg_w, msg_w_count++,
                    delta->percent->in, delta->percent->out,
                    delta->bytes->in, delta->bytes->out);
        }

#ifdef DEBUG
        print_delta_row(new->id, new->name, new->speed, delta);
#endif
    } // foreach(new)

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
    free_delta(delta);

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
