#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <unistd.h>
#include <sys/wait.h>

#include "parser.h"
#include "mutual.h"

// for socket api
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>

int c_getattr(const char *path, struct stat *statbuf)
{
    struct getattr_result res;
    struct mutual_server *st = fuse_get_context()->private_data;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = GETATTR;

    send(st->sfd1, &info, sizeof(struct client_response), 0);
    recv(st->sfd1, &res, sizeof(struct getattr_result), 0);

    memcpy(statbuf, &res.buf, sizeof(struct stat));
    return res.res;
}

int c_mknod(const char *path, mode_t mode, dev_t dev)
{

    int res1, res2;
    struct mutual_server *st = fuse_get_context()->private_data;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = MKNOD;
    info.int1 = mode;

    send(st->sfd1, &info, sizeof(struct client_response), 0);
    send(st->sfd2, &info, sizeof(struct client_response), 0);

    recv(st->sfd1, &res1, sizeof(int), 0);
    recv(st->sfd2, &res2, sizeof(int), 0);

    // printf("----  mknod :path %s  mode %d, result %d\n", path, mode, res1);

    return res1;
}

int c_mkdir(const char *path, mode_t mode)
{

    int res1, res2;
    struct mutual_server *st = fuse_get_context()->private_data;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = MKDIR;
    info.int1 = mode;

    send(st->sfd1, &info, sizeof(struct client_response), 0);
    send(st->sfd2, &info, sizeof(struct client_response), 0);

    recv(st->sfd1, &res1, sizeof(int), 0);
    recv(st->sfd2, &res2, sizeof(int), 0);

    // printf("MKDIR Returned :::: %d   path %s mode %d\n", res1, path, mode);

    return res1;
}

int c_opendir(const char *path, struct fuse_file_info *fi)
{
    struct opendir_result res;
    struct mutual_server *st = fuse_get_context()->private_data;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = OPENDIR;

    send(st->sfd1, &info, sizeof(struct client_response), 0);

    recv(st->sfd1, &res, sizeof(struct opendir_result), 0);
    fi->fh = (intptr_t)res.res_dir;
    int opendir_res = res.res;

    // printf("----  opendir returned : %s res  %d opendirres\n", path, res.res);
    return res.res;
}

int c_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
              struct fuse_file_info *fi)
{
    struct readdir_result res;
    struct mutual_server *st = fuse_get_context()->private_data;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = READDIR;
    info.sent_dir = (DIR *)(uintptr_t)fi->fh;

    send(st->sfd1, &info, sizeof(struct client_response), 0);
    recv(st->sfd1, &res, sizeof(struct readdir_result), 0);
    // printf("READDIRRR   %s     \n", res.buff);
    for (char *token = strtok(res.buff, "/"); token != NULL; token = strtok(NULL, "/"))
    {
        if (filler(buf, token, NULL, 0) != 0)
        {
            printf("----  readdir : %s\n", "ENOMEM");
            return -ENOMEM;
        }
    }

    // printf("READDDIIIRRR %s   res %d\n", res.buff, res.res);

    return res.res;
}

int c_open(const char *path, struct fuse_file_info *fi)
{
    // printf("----  open : %s\n", path);
    struct mutual_server *st = fuse_get_context()->private_data;

    int res1, res2;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = OPEN;
    info.int1 = fi->flags;

    send(st->sfd1, &info, sizeof(struct client_response), 0);
    recv(st->sfd1, &res1, sizeof(int), 0);

    send(st->sfd2, &info, sizeof(struct client_response), 0);
    recv(st->sfd2, &res2, sizeof(int), 0);

    fi->fh = res1;
    map_put(&st->mp, path, res2);

    if (res1 > 0)
        res1 = 0;
    return res1;
}

int c_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    // printf("----  read : %s\n", path);
    struct mutual_server *st = fuse_get_context()->private_data;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = READ;
    info.int1 = fi->fh;
    info.size = size;
    info.off = offset;

    send(st->sfd1, &info, sizeof(struct client_response), 0);
    recv(st->sfd1, buf, size, 0);

    return size;
}

int c_rename(const char *path, const char *newpath)
{
    // printf("----  rename : %s\n", path);
    struct mutual_server *st = fuse_get_context()->private_data;

    int res1, res2;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    memcpy(info.string2, newpath, strlen(newpath) + 1);
    info.func = RENAME;

    send(st->sfd1, &info, sizeof(struct client_response), 0);
    send(st->sfd2, &info, sizeof(struct client_response), 0);
    recv(st->sfd1, &res1, sizeof(int), 0);
    recv(st->sfd2, &res2, sizeof(int), 0);

    // printf("rename Returned :::: %d\n", res1);
    return res1;
}

int c_unlink(const char *path)
{
    // printf("\n\n\n\n\n\n\n\n\n----  unlink : %s\n", path);
    struct mutual_server *st = fuse_get_context()->private_data;
    int res1, res2;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = UNLINK;

    send(st->sfd1, &info, sizeof(struct client_response), 0);
    send(st->sfd2, &info, sizeof(struct client_response), 0);
    recv(st->sfd1, &res1, sizeof(int), 0);
    recv(st->sfd2, &res2, sizeof(int), 0);

    // printf("\n\n\n\n\n\n\n\n\n UNlink Returned :::: %d\n", res1);

    return res1;
}

int c_rmdir(const char *path)
{
    // printf("\n\n\n\n\n\n\n\n\n----  rmdir : %s\n", path);
    struct mutual_server *st = fuse_get_context()->private_data;
    int res1, res2;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = RMDIR;

    send(st->sfd1, &info, sizeof(struct client_response), 0);
    send(st->sfd2, &info, sizeof(struct client_response), 0);
    recv(st->sfd1, &res1, sizeof(int), 0);
    recv(st->sfd2, &res2, sizeof(int), 0);

    // printf("\n\n\n\n\n\n\n\n\nrmdir Returned :::: %d\n", res1);

    return res1;
}

int c_truncate(const char *path, off_t newsize)
{
    // printf("----  truncate : %s\n", path);
    struct mutual_server *st = fuse_get_context()->private_data;
    int res1, res2;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = TRUNCATE;
    info.size = newsize;

    send(st->sfd1, &info, sizeof(struct client_response), 0);
    send(st->sfd2, &info, sizeof(struct client_response), 0);
    recv(st->sfd1, &res1, sizeof(int), 0);
    recv(st->sfd2, &res2, sizeof(int), 0);

    return res1;
}

int c_release(const char *path, struct fuse_file_info *fi)
{
    // printf("----  release : %s\n", path);
    struct mutual_server *st = fuse_get_context()->private_data;
    int res1, res2;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = RELEASE;
    info.sent_dir = (DIR *)(uintptr_t)fi->fh;

    send(st->sfd1, &info, sizeof(struct client_response), 0);
    recv(st->sfd1, &res1, sizeof(int), 0);

    info.sent_dir = (DIR *)(uintptr_t)map_get(&st->mp, path);
    send(st->sfd2, &info, sizeof(struct client_response), 0);
    recv(st->sfd2, &res2, sizeof(int), 0);

    return res1;
}

int c_releasedir(const char *path, struct fuse_file_info *fi)
{
    // printf("----  releasedir : %s\n", path);
    struct mutual_server *st = fuse_get_context()->private_data;

    int res1, res2;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = RELEASEDIR;
    info.sent_dir = (DIR *)(uintptr_t)fi->fh;

    send(st->sfd1, &info, sizeof(struct client_response), 0);
    recv(st->sfd1, &res1, sizeof(int), 0);

    return res1;
}

int c_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    // printf("----  write : %s\n", path);
    struct mutual_server *st = fuse_get_context()->private_data;

    int res1, res2;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = WRITE;
    info.sent_dir = (DIR *)(uintptr_t)fi->fh;
    info.size = size;
    info.off = offset;

    send(st->sfd1, &info, sizeof(struct client_response), 0);
    send(st->sfd1, buf, size, 0);
    recv(st->sfd1, &res1, sizeof(int), 0);

    info.sent_dir = (DIR *)(uintptr_t)map_get(&st->mp, path);
    send(st->sfd2, &info, sizeof(struct client_response), 0);
    send(st->sfd2, buf, size, 0);
    recv(st->sfd2, &res2, sizeof(int), 0);

    // printf("\n releasedir Returned :::: %d\n", res1);

    return res1;
}

struct fuse_operations c_oper = {
    .getattr = c_getattr,
    .mknod = c_mknod,
    .mkdir = c_mkdir,
    .unlink = c_unlink,
    .rmdir = c_rmdir,
    .rename = c_rename,
    .truncate = c_truncate,
    .open = c_open,
    .read = c_read,
    .write = c_write,
    .opendir = c_opendir,
    .readdir = c_readdir,
};

int client(int *sfd, char *address, int i)
{
    // printf("ADDRESS ::: %s   I :::: %d\n", address, i);
    char *ip_address = strtok(address, ":");
    int port = atoi(strtok(NULL, ":"));
    printf("ADDRESS %s   port %d\n", ip_address, port);

    struct sockaddr_in addr;
    int ip;
    char buf[1024];
    *sfd = socket(AF_INET, SOCK_STREAM, 0);
    inet_pton(AF_INET, ip_address, &ip);

    char num[256];

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = ip;

    connect(*sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
}

void logServerConnection(char *serveName, FILE *f)
{
    fwrite(serveName, 1, strlen(serveName), f);
    fwrite(" ", 1, 1, f);
    fwrite("Connected\n", 1, 10, f);
}

int main(int argc, char *argv[])
{
    if (argc != 3 && argc != 4)
    {
        printf("Invalid command format\n");
        return -1;
    }
    int idx = 3;
    if (argc == 3)
        idx = 2;
    FILE *file;
    file = fopen(argv[idx], "r");
    if (file == NULL)
    {
        printf("Invalid config file format\n");
        return -1;
    }

    struct config *cfg = malloc(sizeof(struct config));
    parser_parse(file, cfg);
    fclose(file);

    printf("strgsize %d\n", cfg->storage_size);

    pid_t pid;
    struct mutual_server sts[cfg->storage_size];
    int i = 0;
    for (i; i < cfg->storage_size; i++)
    {
        pid = fork();
        if (pid == 0)
        {
            printf("i ::: %d\n", i);
            map_init(&sts[i].mp);
            memcpy(sts[i].server1, cfg->storage[i]->servers[0], strlen(cfg->storage[i]->servers[0]) + 1);
            memcpy(sts[i].server2, cfg->storage[i]->servers[1], strlen(cfg->storage[i]->servers[1]) + 1);

            FILE *f = fopen(cfg->storage[i]->disk_name, "w");

            client(&sts[i].sfd1, cfg->storage[i]->servers[0], i);
            logServerConnection(sts[i].server1, f);

            client(&sts[i].sfd2, cfg->storage[i]->servers[1], i);
            logServerConnection(sts[i].server2, f);

            memcpy(argv[idx], cfg->storage[i]->mount_point, strlen(cfg->storage[i]->mount_point) + 1);
            fuse_main(argc, argv, &c_oper, &sts[i]);
            break;
        }
    }

    pid_t pid1;
    i = 0;
    while ((pid1 = wait(&i)) > 0)
    {
    }
    i = 0;
    for (i; i < cfg->storage_size; i++)
    {
        fclose(sts[i].fd);
    }
    // for(i=0; i<cfg->storage_size; i++) {
    //     map_dispose(&sts[i].mp);
    // }
    // parser_destroy(cfg);
    return 0;
}
