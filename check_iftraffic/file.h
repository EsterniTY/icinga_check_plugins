#ifndef FILE_H
#define FILE_H

#include "if_status.h"

#define header_id "ITF"
#define header_version 1
#define header_align 1

struct header_t {
    char id[4];
    u_int8_t version;
    char __z1[3];
    u_int16_t records;
    u_int8_t align;
    char __z2[5];
};

void write_info(struct if_status_t *info);
struct if_status_t *read_info(void);

#endif /* FILE_H */
