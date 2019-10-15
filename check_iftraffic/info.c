#include "if_status.h"

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

    if (cell->alias != NULL)
        free(cell->alias);

    if (cell->name != NULL)
        free(cell->name);

    free(cell);

    if (next)
        free_info(next);
}

void add_info(struct if_status_t **root,
              oid id,
              char *name,
              size_t name_len,
              char *alias,
              size_t alias_len,
              ifEntry8_t adminState,
              ifEntry8_t operState,
              ifEntry64_t speed,
              ifEntry64_t inOctets,
              ifEntry64_t outOctets,
              ifEntry32_t inUcastPkts,
              ifEntry32_t outUcastPkts)
{
    struct if_status_t *new;
    struct if_status_t *_root = *root;

    new = (struct if_status_t *)malloc(sizeof(struct if_status_t));
    new->microtime = microtime();
    new->name_len = name_len;
    new->name = (char *)calloc(name_len + 1, sizeof(char));
    memmove(new->name, name, name_len);

    new->alias_len = alias_len;
    new->alias = (char *)calloc(alias_len + 1, sizeof(char));
    memmove(new->alias, alias, alias_len);

    new->speed = speed;
    new->inOctets = inOctets;
    new->outOctets = outOctets;
    new->inUcastPkts = inUcastPkts;
    new->outUcastPkts = outUcastPkts;
    new->adminState = adminState;
    new->operState = operState;
    new->id = id;

    new->next = NULL;

    if (_root != NULL)
        _root->next = new;

    *root = new;
}
