#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "debug.h"

#define size 188

void spacer(const char* str)
{
    char *spacer = malloc(size);
    size_t len = strlen(str);

    spacer[0] = '>';
    memcpy(&spacer[1], str, strlen(str));
    spacer[strlen(str) + 1] = ' ';
    memset(&spacer[len + 2], '-', size - 3 - len);
    spacer[size - 2] = '<';
    spacer[size - 1] = '\0';
    puts(spacer);
    free(spacer);
}
void print_delta_header(void)
{
    printf(format1, "oid", "ifName", "Speed", "deltaTime (s)",
           "inDelta (B)", "outDelta (B)",
           "in bps", "out bps",
           "in pps", "out pps",
           "in Mcast", "out Mcast",
           "in Err", "out Err");
}

void print_delta_row(const oid id, const char *name,
                     const u_int64_t xxx,
                     const mtime_t timeDelta,
                     const u_int64_t inDelta,
                     const u_int64_t outDelta,
                     const u_int64_t in_bps,
                     const u_int64_t out_bps,
                     const u_int64_t in_pps,
                     const u_int64_t out_pps,
                     const u_int64_t in_mcast,
                     const u_int64_t out_mcast,
                     const u_int64_t in_err,
                     const u_int64_t out_err)
{
    printf(format3, id, name, xxx / 1000000, timeDelta,
           inDelta, outDelta,
           in_bps, out_bps,
           in_pps, out_pps,
           in_mcast, out_mcast,
           in_err, out_err);
}

void print_info_table(struct if_status_t *info)
{
    struct if_status_t *curr = NULL;

    printf(format1, "oid", "ifName", "Speed", "MicroTime",
           "inOctets", "outOctets",
           "admStatus", "oprStatus",
           "in pps", "out pps",
           "in Mcast", "out Mcast",
           "in Err", "out Err");

    curr = info;
    while(curr != NULL) {
        printf(format2, curr->id, curr->name,
               curr->speed / 1000000,
               curr->microtime,
               curr->inOctets, curr->outOctets,
               curr->adminState, curr->operState,
               curr->inUcastPkts, curr->outUcastPkts,
               curr->inMcastPkts, curr->outMcastPkts,
               curr->inErrors, curr->outErrors
               );
        curr = curr->next;
    }
}
