#ifndef IFTRAFFIC_DEBUG_H
#define IFTRAFFIC_DEBUG_H

#include "if_status.h"

#define format1 "%3s|%-24s|%5s|%17s|" \
    "%18s|%18s|" \
    "%10s|%10s|" \
    "%14s|%14s|" \
    "%14s|%14s|" \
    "%14s|%14s|" \
    "%6s|%7s\n"
#define format2 "%3lu|%-24s|%'5lu|%'17llu|" \
    "%'18lu|%'18lu|" \
    "%10d|%10d|" \
    "%'14lu|%'14lu|" \
    "%'14lu|%'14lu|" \
    "%'14lu|%'14lu|" \
    "%'6u|%'7u\n"
#define format3 "%3lu|%-24s|%'5lu|%'17llu|" \
    "%'18lu|%'18lu|" \
    "%'10lu|%'10lu|" \
    "%'14lu|%'14lu|" \
    "%'14lu|%'14lu|" \
    "%'14lu|%'14lu|" \
    "%'6lu|%'7lu\n"

void spacer(const char* str);
void print_info_table(struct if_status_t *info);
void print_delta_header(void);
void print_delta_row(const oid id,
                     const char *name,
                     const u_int64_t speed,
                     const struct delta_t *delta);

#endif /* IFTRAFFIC_DEBUG_H */
