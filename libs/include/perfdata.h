#ifndef PERFDATA_H
#define PERFDATA_H

#include <sys/types.h>

#define UOM_NORMAL  0
#define UOM_PERCENT 1
#define UOM_SECONDS 2
#define UOM_BYTES   3
#define UOM_COUNTER 4

typedef u_int8_t uom_t;
typedef u_int64_t bytes_t;
typedef double percent_t;

struct perfdata {
    uom_t uom;

    char *name;
    size_t name_len;

    struct {
        bytes_t value;
        bytes_t warn;
        bytes_t crit;
        bytes_t min;
        bytes_t max;
    } bytes;

    struct {
        percent_t value;
        percent_t warn;
        percent_t crit;
        percent_t min;
        percent_t max;
    } percent;

    struct perfdata *next;
};

void perfdata_add_bytes(struct perfdata **root,
                        char *name, size_t name_len,
                        bytes_t value,
                        bytes_t warn, bytes_t crit,
                        bytes_t min, bytes_t max);

void perfdata_add_normal(struct perfdata **root,
                         char *name, size_t name_len,
                         bytes_t value,
                         bytes_t warn, bytes_t crit,
                         bytes_t min, bytes_t max);

void perfdata_add_percent(struct perfdata **root,
                          char *name, size_t name_len,
                          percent_t value,
                          percent_t warn, percent_t crit,
                          percent_t min, percent_t max);

void perfdata_print(struct perfdata *pf);
void perfdata_free(struct perfdata *pf);

#endif /* PERFDATA_H */
