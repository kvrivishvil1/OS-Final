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

#include "parser.h"
#include "mutual.h"

// for socket api
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>

static const char rootdir[] = "/home/soso/Desktop/rootdir";
int sfd;

int c_getattr(const char *path, struct stat *statbuf)
{
    // printf("----  getattr : %s\n", path);
    struct getattr_result res;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = GETATTR;

    send(sfd, &info, sizeof(struct client_response), 0);
    recv(sfd, &res, sizeof(struct getattr_result), 0);

    memcpy(statbuf, &res.buf, sizeof(struct stat));
    return res.res;
}

int c_mknod(const char *path, mode_t mode, dev_t dev)
{

    int res;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = MKNOD;
    info.int1 = mode;

    send(sfd, &info, sizeof(struct client_response), 0);
    recv(sfd, &res, sizeof(int), 0);

    printf("----  mknod :path %s  mode %d, result %d\n", path, mode, res);

    return res;
}

int c_mkdir(const char *path, mode_t mode)
{

    int res;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = MKDIR;
    info.int1 = mode;

    send(sfd, &info, sizeof(struct client_response), 0);
    recv(sfd, &res, sizeof(int), 0);

    printf("MKDIR Returned :::: %d   path %s mode %d\n", res, path, mode);
    return res;
}

int c_opendir(const char *path, struct fuse_file_info *fi)
{
    struct opendir_result res;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = OPENDIR;

    send(sfd, &info, sizeof(struct client_response), 0);

    recv(sfd, &res, sizeof(struct opendir_result), 0);
    fi->fh = (intptr_t)res.res_dir;
    int opendir_res = res.res;

    printf("----  opendir returned : %s res  %d opendirres\n", path, res.res);
    return res.res;
}

int c_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
              struct fuse_file_info *fi)
{
    struct readdir_result res;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = READDIR;
    info.sent_dir = (DIR *)(uintptr_t)fi->fh;
    printf("%s\n\n\n\n", "");
    send(sfd, &info, sizeof(struct client_response), 0);
    recv(sfd, &res, sizeof(struct readdir_result), 0);
    for (char *token = strtok(res.buff, "/"); token != NULL; token = strtok(NULL, "/"))
    {
        printf("tokeeennn %s\n", token);
        if (filler(buf, token, NULL, 0) != 0)
        {
            printf("----  readdir : %s\n", "ENOMEM");
            return -ENOMEM;
        }
    }
    printf("READDDIIIRRR %s   res %d\n", res.buff, res.res);

    return res.res;
}

int c_open(const char *path, struct fuse_file_info *fi)
{
    printf("----  open : %s\n", path);

    int res;

    struct client_response info;
    strcpy(info.string1, path);
    info.func = OPEN;
    info.int1 = fi->flags;

    send(sfd, &info, sizeof(struct client_response), 0);
    recv(sfd, &res, sizeof(int), 0);

    printf("\n\n\n\n\n\n\n\n\n open Returned :::: %d\n", res);

    fi->fh = res;
    if (res > 0)
        res = 0;
    return res;
}

int c_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    printf("----  read : %s\n", path);

    struct client_response info;
    strcpy(info.string1, path);
    info.func = READ;
    info.int1 = fi->fh;
    info.size = size;
    info.off = offset;

    send(sfd, &info, sizeof(struct client_response), 0);
    recv(sfd, buf, size, 0);

    return size;
}

int c_rename(const char *path, const char *newpath)
{
    printf("----  rename : %s\n", path);

    int res;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    memcpy(info.string2, newpath, strlen(newpath) + 1);
    info.func = RENAME;

    send(sfd, &info, sizeof(struct client_response), 0);
    recv(sfd, &res, sizeof(int), 0);

    printf("rename Returned :::: %d\n", res);
    return res;
}

int c_unlink(const char *path)
{
    printf("\n\n\n\n\n\n\n\n\n----  unlink : %s\n", path);
    int res;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = UNLINK;

    send(sfd, &info, sizeof(struct client_response), 0);
    recv(sfd, &res, sizeof(int), 0);

    printf("\n\n\n\n\n\n\n\n\n UNlink Returned :::: %d\n", res);

    return res;
}

int c_rmdir(const char *path)
{
    printf("\n\n\n\n\n\n\n\n\n----  rmdir : %s\n", path);
    int res;

    struct client_response info;
    strcpy(info.string1, path);
    info.func = RMDIR;

    send(sfd, &info, sizeof(struct client_response), 0);
    recv(sfd, &res, sizeof(int), 0);

    printf("\n\n\n\n\n\n\n\n\nrmdir Returned :::: %d\n", res);

    return res;
}

int c_truncate(const char *path, off_t newsize)
{
    printf("----  truncate : %s\n", path);
    int res;

    struct client_response info;
    strcpy(info.string1, path);
    info.func = TRUNCATE;
    info.size = newsize;

    send(sfd, &info, sizeof(struct client_response), 0);
    recv(sfd, &res, sizeof(int), 0);

    printf("\n\n\n\n\n\n\n\n\truncate Returned :::: %d\n", res);
    return res;
}

int c_release(const char *path, struct fuse_file_info *fi)
{
    printf("----  release : %s\n", path);
    int res;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = RELEASE;
    info.sent_dir = (DIR *)(uintptr_t)fi->fh;

    send(sfd, &info, sizeof(struct client_response), 0);
    recv(sfd, &res, sizeof(int), 0);

    printf("\n\n\n\n\n\n\n\n\n release Returned :::: %d\n", res);

    return res;
}

int c_releasedir(const char *path, struct fuse_file_info *fi)
{
    printf("----  releasedir : %s\n", path);

    int res;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = RELEASE;
    info.sent_dir = (DIR *)(uintptr_t)fi->fh;

    send(sfd, &info, sizeof(struct client_response), 0);
    recv(sfd, &res, sizeof(int), 0);

    printf("\n\n\n\n\n\n\n\n\n releasedir Returned :::: %d\n", res);

    return res;
}

int c_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    printf("----  write : %s\n", path);

    int res;

    struct client_response info;
    memcpy(info.string1, path, strlen(path) + 1);
    info.func = WRITE;
    info.sent_dir = (DIR *)(uintptr_t)fi->fh;
    info.size = size;
    info.off = offset;

    send(sfd, &info, sizeof(struct client_response), 0);
    send(sfd, buf, size, 0);

    recv(sfd, &res, sizeof(int), 0);

    printf("\n releasedir Returned :::: %d\n", res);

    return res;
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

int client(char *address)
{
    char *token;

    struct sockaddr_in addr;
    int ip;
    char buf[1024];
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    inet_pton(AF_INET, "127.0.0.1", &ip);

    token = strtok(NULL, ":"); //PORT

    char num[256];
    int port = 10001;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = ip;

    connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    char *test = "darooo";
    printf("sfd ::::: %d", sfd);
    write(sfd, test, 100);
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Invalid command format\n");
        return -1;
    }

    int argv_n = 3;
    // if (argc == 4)
    //     argv_n = 3;
    printf("Fiename :: %s\n", argv[argv_n]);

    FILE *file;
    file = fopen(argv[argv_n], "r");
    if (file == NULL)
    {
        printf("Invalid config file format\n");
        return -1;
    }

    struct config *cfg = malloc(sizeof(struct config));
    parser_parse(file, cfg);
    fclose(file);

    parser_print(cfg);
    strcpy(argv[argv_n], cfg->storage[0]->mount_point);
    client(argv[argv_n]);

    return fuse_main(argc, argv, &c_oper, NULL);
}
