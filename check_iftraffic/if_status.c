#include "if_status.h"
#include "check_iftraffic.h"

#define DELTA(a, b) ((a)>(b) ? (OCTET_MAX_VALUE-a+b) : (b-a))

struct opt_s options;

void set_delta(struct delta_t *delta,
               struct if_status_t *old,
               struct if_status_t *new)
{
    mtime_t time = (new->microtime - old->microtime) / 1000;

    if (time == 0)
        return;

    delta->bytes->in    = (DELTA(old->inOctets,    new->inOctets ) * 8)  / time;
    delta->bytes->out   = (DELTA(old->outOctets,   new->outOctets) * 8)  / time;
    delta->errors->in   = DELTA(old->inErrors,     new->inErrors)        / time;
    delta->errors->out  = DELTA(old->outErrors,    new->outErrors)       / time;
    delta->mcast->in    = DELTA(old->inMcastPkts,  new->inMcastPkts)     / time;
    delta->mcast->out   = DELTA(old->outMcastPkts, new->outMcastPkts)    / time;
    delta->bcast->in    = DELTA(old->inBcastPkts,  new->inBcastPkts)     / time;
    delta->bcast->out   = DELTA(old->outBcastPkts, new->outBcastPkts)    / time;
    delta->packets->in  = DELTA(old->inUcastPkts,  new->inUcastPkts)     / time;
    delta->packets->out = DELTA(old->outUcastPkts, new->outUcastPkts)    / time;

    delta->packets->in  += delta->mcast->in  + delta->bcast->in;
    delta->packets->out += delta->mcast->out + delta->bcast->out;

    if (new->speed > 0) {
        delta->percent->in  = (double)delta->bytes->in  * 100 / new->speed;
        delta->percent->out = (double)delta->bytes->out * 100 / new->speed;
    }

    delta->time = time;
#ifdef DEBUG
    delta->octets->in  = (DELTA(old->inOctets,    new->inOctets ) * 8);
    delta->octets->out = (DELTA(old->outOctets,   new->outOctets) * 8);
    delta->old         = old;
    delta->new         = new;
#endif
}

struct delta_t *init_delta(void)
{
    struct delta_t *delta = calloc(1, sizeof(struct delta_t));

#ifdef DEBUG
    delta->octets  = calloc(1, sizeof(struct in_out_t));
#endif
    delta->bytes   = calloc(1, sizeof(struct in_out_t));
    delta->packets = calloc(1, sizeof(struct in_out_t));
    delta->mcast   = calloc(1, sizeof(struct in_out_t));
    delta->bcast   = calloc(1, sizeof(struct in_out_t));
    delta->errors  = calloc(1, sizeof(struct in_out_t));
    delta->percent = calloc(1, sizeof(struct in_out_float_t));

    return delta;
}

void free_delta(struct delta_t *delta)
{
#ifdef DEBUG
    free(delta->octets);
#endif
    free(delta->bytes);
    free(delta->packets);
    free(delta->mcast);
    free(delta->bcast);
    free(delta->errors);
    free(delta->percent);

    free(delta);
}
