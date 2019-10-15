#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "perfdata.h"

void _int(uom_t uom, struct perfdata **root,
                      char *name, size_t name_len,
                      bytes_t value,
                      bytes_t warn, bytes_t crit,
                      bytes_t min, bytes_t max)
{
    struct perfdata *_root = *root;
    struct perfdata *new = calloc(1, sizeof(struct perfdata));

    new->uom = uom;
    new->name_len = name_len;
    new->name = (char *)calloc(name_len + 1, sizeof(char));
    memcpy(new->name, name, name_len);

    new->bytes.value = value;
    new->bytes.warn = warn;
    new->bytes.crit = crit;
    new->bytes.min = min;
    new->bytes.max = max;

    new->next = NULL;

    if (_root != NULL)
        _root->next = new;

    *root = new;
}

void perfdata_add_bytes(struct perfdata **root,
                        char *name, size_t name_len,
                        bytes_t value,
                        bytes_t warn, bytes_t crit,
                        bytes_t min, bytes_t max)
{
    _int(UOM_BYTES, root, name, name_len, value, warn, crit, min, max);
}

void perfdata_add_normal(struct perfdata **root,
                         char *name, size_t name_len,
                         bytes_t value,
                         bytes_t warn, bytes_t crit,
                         bytes_t min, bytes_t max)
{
    _int(UOM_NORMAL, root, name, name_len, value, warn, crit, min, max);
}

void perfdata_add_percent(struct perfdata **root,
                          char *name, size_t name_len,
                          percent_t value,
                          percent_t warn, percent_t crit,
                          percent_t min, percent_t max)
{
    struct perfdata *_root = *root;
    struct perfdata *new = calloc(1, sizeof(struct perfdata));

    new->uom = UOM_PERCENT;
    new->name_len = name_len;
    new->name = (char *)calloc(name_len + 1, sizeof(char));
    memcpy(new->name, name, name_len);

    new->percent.value = value;
    new->percent.warn = warn;
    new->percent.crit = crit;
    new->percent.min = min;
    new->percent.max = max;

    new->next = NULL;

    if (_root != NULL)
        _root->next = new;

    *root = new;
}

void perfdata_print(struct perfdata *pf)
{
    struct perfdata *curr = pf;

#ifndef DEBUG
    if (pf != NULL)
        printf(" | ");
#endif

    while (curr != NULL) {
        switch (curr->uom) {
        case UOM_BYTES:
            printf("'%s'=%luB;%lu;%lu;%lu;%lu", curr->name,
                   curr->bytes.value,
                   curr->bytes.warn, curr->bytes.crit,
                   curr->bytes.min, curr->bytes.max);
            break;
        case UOM_PERCENT:
            printf("'%s'=%.2f%%;%.0f;%.0f;%.0f;%.0f", curr->name,
                   curr->percent.value,
                   curr->percent.warn, curr->percent.crit,
                   curr->percent.min, curr->percent.max);
            break;
        case UOM_NORMAL:
            printf("'%s'=%lu;%lu;%lu;%lu;%lu", curr->name,
                   curr->bytes.value,
                   curr->bytes.warn, curr->bytes.crit,
                   curr->bytes.min, curr->bytes.max);
            break;
        }

        if (curr->next)
#ifdef DEBUG
            printf("\n");
#else
            printf(" ");
#endif
        else
            puts("");

        curr = curr->next;
    }
}

void perfdata_free(struct perfdata *pf)
{
    struct perfdata *next = pf->next;

    if (pf->name)
        free(pf->name);

    free(pf);

    if (next)
        perfdata_free(next);
}
