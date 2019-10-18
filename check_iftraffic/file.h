#ifndef FILE_H
#define FILE_H

#include "if_status.h"

#define header_id      "ITF"
#define header_version 9
#define header_align   1

#define HEADER_FLAG_ALIGN     0x01
#define HEADER_FLAG_IFSPEED64 0x02
#define HEADER_FLAG_VERSION1  0x04

struct header_t {
    char      id[4];
    u_int8_t  version;
    char      __z1[3];
    u_int16_t records;
    u_int64_t flags;
    char      __z2[2];
    u_int     uptime;
};

void write_info(struct if_status_t *info);
struct if_status_t *read_info(void);

#endif /* FILE_H */
