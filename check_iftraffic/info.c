#include "if_status.h"
#include "ifsnmp.h"

#include "info.h"
#include "snmp.h"
#include "check_iftraffic.h"

struct opt_s options;
struct host_settings_s host_settings;

void free_info(struct if_status_t *cell)
{
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

void fill_info(struct if_status_t **curr, const struct variable_list *vars,
               char *name, size_t name_len)
{
    oid curr_oid = vars->name[vars->name_length-1];
    u_int64_t speed = 0;
    u_int64_t inOctets = 0;
    u_int64_t outOctets = 0;
    u_int8_t operState = ifEntry8(oid_ifOperStatus, curr_oid);
    u_int8_t adminState = ifEntry8(oid_ifAdminStatus, curr_oid);

    if (options.downstate == 0)
        if (operState != IF_OPER_UP || adminState != IF_ADMIN_UP)
            return;

    speed = ifEntry64(oid_ifSpeed64, curr_oid);

    if (speed == 0) {
        speed = ifEntry32(oid_ifSpeed32, curr_oid);
    } else
        speed = speed * IF_SPEED_1MB;

    if (speed < IF_SPEED_20MB || host_settings.has_ifSpeed64 == 0) {
        inOctets = ifEntry32(oid_ifInOctets32, curr_oid);
        outOctets = ifEntry32(oid_ifOutOctets32, curr_oid);
    } else {
        inOctets = ifEntry64(oid_ifInOctets64, curr_oid);
        if (errstat() != SNMP_ERR_NOERROR) {
            host_settings.has_ifSpeed64 = 0;
            inOctets = ifEntry32(oid_ifInOctets32, curr_oid);
        }

        outOctets = ifEntry64(oid_ifOutOctets64, curr_oid);
        if (errstat() != SNMP_ERR_NOERROR) {
            host_settings.has_ifSpeed64 = 0;
            outOctets = ifEntry32(oid_ifOutOctets32, curr_oid);
        }
    }

    add_info(curr, curr_oid,
             name,
             name_len,
             adminState,
             operState,
             speed,
             inOctets,
             outOctets
             );
}
