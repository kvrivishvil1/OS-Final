struct map
{
    char **keys;
    int *values;
    int alloc_len;
    int log_len;
};

void map_init(struct map *mp);

void map_dispose(struct map *mp);

void map_put(struct map *mp, const char *key, int value);

int map_get(struct map *mp, const char *key);

void map_print(struct map *mp);
