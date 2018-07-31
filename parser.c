#include "parser.h"

void parser_init(struct config *cfg)
{
    assert(cfg != NULL);
    cfg->storage_size = 0;
    cfg->storage = NULL;
    cfg->error_log = NULL;
    cfg->cache_replacment = NULL;
}

void storage_destroy(struct storage *strg)
{
    free(strg->disk_name);
    free(strg->mount_point);
    int i = 0;
    for (i; i < strg->server_num; i++)
        free(strg->servers[i]);
    free(strg->servers);
    free(strg->hot_swap);
}

void parser_destroy(struct config *cfg)
{
    assert(cfg != NULL);
    int i = 0;
    for (i; i < cfg->storage_size; i++)
    {
        struct storage *strg = cfg->storage[i];
        storage_destroy(strg);
        free(strg);
    }
    if (cfg->storage != NULL)
        free(cfg->storage);
    if (cfg->error_log != NULL)
        free(cfg->error_log);
    if (cfg->cache_replacment != NULL)
        free(cfg->cache_replacment);
    free(cfg);
}

char *make_place(int size)
{
    char *place = malloc(size);
    assert(place != NULL);
    return place;
}

void copy_strings(char *src, char **dest)
{
    *dest = make_place(strlen(src) + 1);
    memcpy(*dest, src, strlen(src) + 1);
}

int read_storages(FILE *config_file, struct config *cfg)
{
    struct storage *strg = malloc(sizeof(struct storage));
    strg->server_num = 0;
    char *disk_name = make_place(300);
    char *mount_point = make_place(300);
    char *servers = make_place(300);
    char *hot_swap = make_place(300);
    int res = fscanf(config_file,
                     " diskname = %s mountpoint = %s raid = %d servers = %[^\n] hotswap = %s",
                     disk_name, mount_point, &strg->raid, servers, hot_swap);
    if (res != 5)
    {
        free(disk_name);
        free(mount_point);
        free(servers);
        free(hot_swap);
        free(strg);
        return 0;
    }
    copy_strings(disk_name, &strg->disk_name);
    copy_strings(mount_point, &strg->mount_point);
    copy_strings(hot_swap, &strg->hot_swap);

    strg->servers = malloc(0);
    for (char *token = strtok(servers, ", "); token != NULL; token = strtok(NULL, ", "))
    {
        strg->servers = realloc(strg->servers, (strg->server_num + 1) * sizeof(char *));
        copy_strings(token, &strg->servers[strg->server_num]);
        strg->server_num++;
    }
    free(disk_name);
    free(mount_point);
    free(servers);
    free(hot_swap);
    cfg->storage = realloc(cfg->storage, (cfg->storage_size + 1) * sizeof(struct storage *));
    memcpy(&cfg->storage[cfg->storage_size], &strg, sizeof(struct storage *));
    cfg->storage_size++;
    return 1;
}

void read_config(FILE *config_file, struct config *cfg)
{
    char *error_log = make_place(300);
    char *cache_replacment = make_place(300);
    int res = fscanf(config_file,
                     "errorlog = %s cache_size = %dM cache_replacment = %s timeout = %d",
                     error_log, &cfg->cache_size, cache_replacment, &cfg->timeout);
    assert(res == 4);
    copy_strings(error_log, &cfg->error_log);
    copy_strings(cache_replacment, &cfg->cache_replacment);
    free(error_log);
    free(cache_replacment);
}

void parser_parse(FILE *config_file, struct config *cfg)
{
    assert(cfg != NULL);
    assert(config_file != NULL);
    read_config(config_file, cfg);
    int res = 1;
    for(res; res != 0; res = read_storages(config_file, cfg));
}

void parser_print(struct config *cfg)
{
    printf("Error Log : %s\n", cfg->error_log);
    printf("Cache Size : %d\n", cfg->cache_size);
    printf("Cache Replacement : %s\n", cfg->cache_replacment);
    printf("Timeout : %d\n", cfg->timeout);
    printf("Storage Size : %d\n", cfg->storage_size);
    int i = 0;
    for (i; i < cfg->storage_size; i++)
    {
        printf("Server N: %d  Disk Name : %s\n", i + 1, cfg->storage[i]->disk_name);
        printf("Server N: %d  Mount Point : %s\n", i + 1, cfg->storage[i]->mount_point);
        printf("Server N: %d  Raid : %d\n", i + 1, cfg->storage[i]->raid);
        printf("Server N: %d  Servers Count : %d\n", i + 1, cfg->storage[i]->server_num);
        int j = 0;
        for (j; j < cfg->storage[i]->server_num; j++)
            printf("Server N: %d  adrress %d = %s\n", i + 1, j + 1, cfg->storage[i]->servers[j]);
        printf("Server N: %d  Hot Swap : %s\n", i + 1, cfg->storage[i]->hot_swap);
    }
}