#ifndef IFTRAFFIC_DEBUG_H
#define IFTRAFFIC_DEBUG_H

#include "if_status.h"

#define format1 "%4s %-24s %15s %20s %20s %20s %15s %15s %14s %14s %10s %10s\n"
#define format2 "%4lu %-24s %'15lu %'20llu %'20lu %'20lu %15d %15d %'14lu %'14lu %'10u %'10u\n"
#define format3 "%4lu %-24s %'15lu %'20llu %'20lu %'20lu %'15lu %'15lu %'14lu %'14lu %'10lu %'10lu\n"

void spacer(const char* str);
void print_info_table(struct if_status_t *info);
void print_delta_row(const oid id, const char *name,
                     const u_int64_t xxx,
                     const mtime_t timeDelta,
                     const u_int64_t inDelta,
                     const u_int64_t outDelta,
                     const u_int64_t in_bps,
                     const u_int64_t out_bps,
                     const u_int64_t in_pps,
                     const u_int64_t out_pps,
                     const u_int64_t in_err,
                     const u_int64_t out_err);

#endif /* IFTRAFFIC_DEBUG_H */
