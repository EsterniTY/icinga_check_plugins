#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "debug.h"

#define size 218

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
           "in Bcast", "out Bcast",
           "in Err", "out Err");
}

void print_delta_row(const oid id,
                     const char *name,
                     const u_int64_t speed,
                     const struct delta_t *delta)
{
    printf(format3, id, name, speed / 1000000, delta->time,
       #ifdef DEBUG
           delta->octets->in, delta->octets->out,
       #else
           (u_int64_t)0, (u_int64_t)0,
       #endif
           delta->bytes->in, delta->bytes->out,
           delta->packets->in, delta->packets->out,
           delta->mcast->in, delta->mcast->out,
           delta->bcast->in, delta->bcast->out,
           delta->errors->in, delta->errors->out
           );
}

void print_info_table(struct if_status_t *info)
{
    struct if_status_t *curr = NULL;

    printf(format1, "oid", "ifName", "Speed", "MicroTime",
           "inOctets", "outOctets",
           "admStatus", "oprStatus",
           "in pps", "out pps",
           "in Mcast", "out Mcast",
           "in Bcast", "out Bcast",
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
               curr->inBcastPkts, curr->outBcastPkts,
               curr->inErrors, curr->outErrors
               );
        curr = curr->next;
    }
}
