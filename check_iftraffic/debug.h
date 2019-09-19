#ifndef IFTRAFFIC_DEBUG_H
#define IFTRAFFIC_DEBUG_H

#include "if_status.h"

#define format1 "%4s %-44s %15s %20s %20s %20s %15s %15s\n"
#define format2 "%4lu %-44s %'15lu %'20llu %'20lu %'20lu %15d %15d\n"
#define format3 "%4lu %-44s %'15lu %'20llu %'20lu %'20lu %'15lu %'15lu\n"

void spacer(const char* str);
void print_info_table(struct if_status_t *info);
void print_delta_row(const oid id, const char *name,
                     const u_int64_t xxx,
                     const mtime_t timeDelta,
                     const u_int64_t inDelta,
                     const u_int64_t outDelta,
                     const u_int64_t in_bps,
                     const u_int64_t out_bps);

#endif /* IFTRAFFIC_DEBUG_H */
