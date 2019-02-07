#include <sys/types.h>
#include <sys/stat.h>
#include "utils.h"
#include "check_iftraffic.h"
#include "info.h"
#include "file.h"

struct opt_s options;

size_t fw(const void *ptr, const size_t size, FILE *fp)
{
    fwrite(ptr, size, 1, fp);
    return size;
}

size_t fr(void *ptr, const size_t size, FILE *fp)
{
    size_t c = fread(ptr, size, 1, fp);
    return c * size;
}

void write_header(FILE *fp, u_int16_t records)
{
    struct header_t header;

    memcpy(header.id, header_id, sizeof(header.id));
    header.version = header_version;
    header.records = records;
    header.align = header_align;
    memset(header.__z1, '\0', sizeof(header.__z1));
    memset(header.__z2, '\0', sizeof(header.__z2));

    fwrite(&header, sizeof(struct header_t), 1, fp);
}

struct if_status_t *read_info()
{
    struct stat st = {0};
    if (stat(options.cache_path, &st) == -1)
        return NULL;

    FILE *fp = fopen(options.cache_path, "rb");

    if (fp == NULL) {
        char err[128];
        snprintf(err, 128, "Unable to read cache file for %s from %s",
                 options.host, options.cache_dir);
        exit_error(EXIT_UNKNOWN, err);
    }

    struct header_t header;
    fread(&header, sizeof(header), 1, fp);

    if (strcmp(header_id, header.id) != 0 ||
            header.version != header_version) {
        remove(options.cache_path);

        return NULL;
    }

    struct if_status_t *first = NULL;
    struct if_status_t *curr = NULL;

    for (u_int16_t i = 0; i < header.records; i++) {
        struct if_status_t *prev = curr;
        curr = malloc(sizeof(struct if_status_t));
        int len = 0;

        len += fr(&curr->microtime, sizeof(curr->microtime), fp);
        len += fr(&curr->id, sizeof(curr->id), fp);
        len += fr(&curr->speed, sizeof(curr->speed), fp);
        len += fr(&curr->inOctets, sizeof(curr->inOctets), fp);
        len += fr(&curr->outOctets, sizeof(curr->outOctets), fp);
        len += fr(&curr->operState, sizeof(curr->operState), fp);
        len += fr(&curr->adminState, sizeof(curr->adminState), fp);
        len += fr(&curr->name_len, sizeof(curr->name_len), fp);
        curr->name = malloc(curr->name_len + 1);
        len += fr(curr->name, curr->name_len, fp);
        curr->name[curr->name_len] = '\0';

        curr->next = NULL;

        if (!first)
            first = curr;

        if (prev)
            prev->next = curr;

        if (header.align && (len % 16))
            fseek(fp, 16 - (len % 16), SEEK_CUR);
    }

    fclose(fp);

    return first;
}

void write_info(struct if_status_t *info)
{
    struct if_status_t *curr;

    struct stat st = {0};
    if (stat(options.cache_dir, &st) == -1) {
        mkdir(options.cache_dir, 0700);
    }

    FILE *fp = fopen(options.cache_path, "wb");

    if (fp == NULL) {
        char err[128];
        snprintf(err, 128, "Unable to write cache file for %s into %s",
                 options.host, options.cache_dir);
        exit_error(EXIT_UNKNOWN, err);
    }

    {
        size_t len = sizeof(struct header_t);
        char *buf = malloc(len);
        memset(buf, '\0', len);
        fwrite(buf, len, 1, fp);
        free(buf);
    }

    curr = info;
    u_int16_t records = 0;

    while(curr != NULL) {
        size_t len = 0;
        len += fw(&curr->microtime, sizeof (curr->microtime), fp);
        len += fw(&curr->id, sizeof(curr->id), fp);
        len += fw(&curr->speed, sizeof(curr->speed), fp);
        len += fw(&curr->inOctets, sizeof(curr->inOctets), fp);
        len += fw(&curr->outOctets, sizeof(curr->outOctets), fp);
        len += fw(&curr->adminState, sizeof(curr->adminState), fp);
        len += fw(&curr->operState, sizeof(curr->operState), fp);
        len += fw(&curr->name_len, sizeof(curr->name_len), fp);
        len += fw(curr->name, curr->name_len, fp);

        if (header_align && (len % 16)) {
            u_int8_t l = 16 - (len % 16);
            char *buf = malloc(l);
            memset(buf, '\0', l);
            len += fw(buf, l, fp);
            free(buf);
        }

        curr = curr->next;
        records++;
    }

    rewind(fp);
    write_header(fp, records);

    fclose(fp);
}
