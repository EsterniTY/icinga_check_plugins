#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>

typedef u_int8_t code_t;

#define EXIT_OK 0
#define EXIT_WARNING 1
#define EXIT_CRITICAL 2
#define EXIT_UNKNOWN 3

int exit_ok(const code_t code, const char *msg, const char *perfdata);
int exit_error(const code_t code, const char *message);

size_t format_perfdata(char *perfdata,
                       const char *label,
                       const u_int val,
                       const u_int warn,
                       const u_int crit);
void fix_threshold(u_int8_t *warn, u_int8_t *crit);

#endif /* UTILS_H */
