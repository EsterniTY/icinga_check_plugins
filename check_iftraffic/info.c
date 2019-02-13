#include "if_status.h"
#include "ifsnmp.h"

#include "info.h"
#include "snmp.h"
#include "check_iftraffic.h"

struct opt_s options;
struct host_settings_s host_settings;

void free_info(struct if_status_t *cell)
{
    if (cell == NULL)
        return;

    struct if_status_t *next = cell->next;

    if (cell->name)
        free(cell->name);

    free(cell);

    if (next)
        free_info(next);
}

void add_info(struct if_status_t **root,
              oid id,
              char *name,
              size_t name_len,
              u_int8_t adminState,
              u_int8_t operState,
              u_int64_t speed,
              u_int64_t inOctets,
              u_int64_t outOctets)
{
    struct if_status_t *new;
    struct if_status_t *_root = *root;

    new = (struct if_status_t *)malloc(sizeof(struct if_status_t));
    new->microtime = microtime();
    new->name_len = name_len;
    new->name = malloc(new->name_len + 1);
    memcpy(new->name, name, new->name_len);
    new->name[new->name_len] = '\0';

    new->speed = speed;
    new->inOctets = inOctets;
    new->outOctets = outOctets;
    new->adminState = adminState;
    new->operState = operState;
    new->id = id;

    new->next = NULL;

    if (_root != NULL)
        _root->next = new;

    *root = new;
}
