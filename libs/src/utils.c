#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

static char messages[4][9] = {"OK", "WARNING", "CRITICAL", "UNKNOWN"};

int exit_ok(const code_t code, const char *msg, const char *perfdata)
{
    printf("%s: %s|%s\n", messages[code], msg, perfdata);

    exit(code);
}

int exit_error(const code_t code, const char *message)
{
    if (code == EXIT_OK)
        printf("OK: %s\n", message);
    else
        printf("CRITICAL: %s\n", message);

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
