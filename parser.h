#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct storage{
    char * disk_name; 
    char * mount_point;
    int raid;
    int server_num;
    char ** servers;
    char * hot_swap;
    int sfd1;
    int sfd2;
    int res2;
};

struct config{
    char * error_log;
    int cache_size;
    char * cache_replacment;
    int timeout;
    int storage_size;
    struct storage ** storage;
};

void parser_init(struct config * cfg);
void parser_parse (FILE * config_file, struct config * cfg);
void parser_destroy(struct config * cfg);
void parser_print(struct config *cfg);