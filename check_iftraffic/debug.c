#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "debug.h"

void spacer(const char* str)
{
    char *spacer = malloc(191);
    size_t len = strlen(str);

    memset(&spacer[len + 2], '-', 188 - len);
    memcpy(&spacer[1], str, strlen(str));
    spacer[0] = '>';
    spacer[strlen(str) + 1] = ' ';
    spacer[189] = '<';
    spacer[190] = '\0';
    puts(spacer);
    free(spacer);
}

void print_delta_row(const oid id, const char *name,
                     const u_int64_t xxx,
                     const mtime_t timeDelta,
                     const u_int64_t inDelta,
                     const u_int64_t outDelta,
                     const u_int64_t in_bps,
                     const u_int64_t out_bps,
                     const u_int64_t in_pps,
                     const u_int64_t out_pps)
{
    printf(format3, id, name, xxx, timeDelta, inDelta, outDelta,
           in_bps, out_bps, in_pps, out_pps);
}

void print_info_table(struct if_status_t *info)
{
    struct if_status_t *curr = NULL;

    printf(format1, "oid", "ifName",
           "Speed", "MicroTime",
           "inOctets", "outOctets", "adminStatus", "operStatus", "in pps", "out pps");

    curr = info;
    while(curr != NULL) {
        printf(format2, curr->id, curr->name,
               curr->speed,
               curr->microtime,
               curr->inOctets, curr->outOctets,
               curr->adminState, curr->operState,
               curr->inUcastPkts, curr->outUcastPkts
               );
        curr = curr->next;
    }
}
