#include "map.h"
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

void map_init(struct map *mp)
{
    mp->alloc_len = 1;
    mp->log_len = 0;
    mp->keys = (char **)malloc(mp->alloc_len * sizeof(char *));
    mp->values = (int *)malloc(mp->alloc_len * sizeof(int));
}

void map_dispose(struct map *mp)
{
    for (int i = 0; i < mp->log_len; i++)
    {
        free(mp->keys[i]);
    }
    free(mp->keys);
    free(mp->values);
}

void grow(struct map *mp)
{
    mp->keys = realloc(mp->keys, mp->alloc_len * sizeof(char *) * 2);
    mp->values = realloc(mp->values, mp->alloc_len * sizeof(int *) * 2);
    mp->alloc_len *= 2;
}

void map_put(struct map *mp, const char *key, int value)
{
    int i;
    for (i = 0; i < mp->log_len; i++)
    {
        if (strcmp(key, mp->keys[i]) == 0)
        {
            // free(mp->values[i]);
            // mp->values[i] = strdup(value);
            mp->values[i] = value;
            return;
        }
    }

    if (mp->log_len >= mp->alloc_len)
    {
        grow(mp);
    }
    mp->keys[mp->log_len] = strdup(key);
    mp->values[mp->log_len] = value;
    mp->log_len++;
}

int map_get(struct map *mp, const char *key)
{
    int i;
    for (i = 0; i < mp->log_len; i++)
    {
        if (strcmp(key, mp->keys[i]) == 0)
        {
            return mp->values[i];
        }
    }
    return -1;
}

void map_print(struct map *mp)
{
    int i;
    for (i = 0; i < mp->log_len; i++)
    {
        printf("%s %d\n", mp->keys[i], mp->values[i]);
    }
}