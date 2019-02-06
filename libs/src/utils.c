#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "utils.h"

static char messages[4][9] = {"OK", "WARNING", "CRITICAL", "UNKNOWN"};

int exit_ok(const code_t code, const char *msg, const char *perfdata)
{
    printf("%s: %s|%s\n", messages[code], msg, perfdata);

    exit(code);
}

int exit_error(const code_t code, const char *message)
{
    printf("%s: %s\n", messages[code], message);

    exit(code);
}

size_t format_perfdata(char *perfdata,
                       const char *label,
                       const u_int val,
                       const u_int warn,
                       const u_int crit)
{
    char perf[50];
    size_t size = 0;

    if (warn == 0 && crit == 0)
        sprintf(perf, "'%s'=%d;;;;", label, val);
    else if (warn == 0)
        sprintf(perf, "'%s'=%d;%d;;;", label, val, warn);
    else if (crit == 0)
        sprintf(perf, "'%s'=%d;;%d;;", label, val, crit);
    else
        sprintf(perf, "'%s'=%d;%d;%d;;", label, val, warn, crit);

    size = (size_t)strlen(perf);
    memcpy(perfdata, perf, size);
    perfdata[size] = 0;

    return size;
}

void fix_threshold(u_int8_t *warn, u_int8_t *crit)
{
    if ((*warn != 0 && *crit != 0) && *warn > *crit) {
        u_int8_t tmp = *warn;
        *warn = *crit;
        *crit = tmp;
    }
}

mtime_t microtime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    mtime_t microtime = (mtime_t)(tv.tv_sec) * 1000 +
            (mtime_t)(tv.tv_usec) / 1000;

    return microtime;
}
