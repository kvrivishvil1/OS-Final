#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include <string.h>
#include <stdlib.h>

#include <fcntl.h>
#include "mutual.h"

#include <errno.h>

#define BACKLOG 10

int sfd, cfd;
char server_path[] = "/home/soso/Desktop/serverdir";

void s_getattr(struct client_response *info)
{
    struct getattr_result res;

    char full_path[512];
    memcpy(&full_path, server_path, strlen(server_path) + 1);
    memcpy(&full_path[strlen(server_path)], info->string1, strlen(info->string1) + 1);

    res.res = lstat(full_path, &(res.buf));
    if (res.res < 0)
        res.res = -errno;

    // printf("GETATTR %d\n", res.res);
    send(cfd, &res, sizeof(struct getattr_result), 0);
}

void s_mknod(struct client_response *info)
{
    int res;

    char full_path[512];
    memcpy(&full_path, server_path, strlen(server_path) + 1);
    memcpy(&full_path[strlen(server_path)], info->string1, strlen(info->string1) + 1);

    res = open(full_path, O_CREAT | O_EXCL | O_WRONLY, info->int1);
    if (res < 0)
        res = -errno;

    if (res >= 0)
        res = close(res);
    printf("MKNODDD    path %s  mode %d\n", full_path, info->int1);
    send(cfd, &res, sizeof(int), 0);
}

int s_mkdir(struct client_response *info)
{
    int res;

    char full_path[512];
    memcpy(&full_path, server_path, strlen(server_path) + 1);
    memcpy(&full_path[strlen(server_path)], info->string1, strlen(info->string1) + 1);

    res = mkdir(full_path, info->int1);
    printf("MKDIR    path %s  mode %d\n", full_path, info->int1);
    
    if (res < 0)
        res = -errno;

    send(cfd, &res, sizeof(int), 0);
}

void s_opendir(struct client_response *info)
{
    struct opendir_result res;

    char full_path[512];
    memcpy(&full_path, server_path, strlen(server_path) + 1);
    memcpy(&full_path[strlen(server_path)], info->string1, strlen(info->string1) + 1);
    
    res.res_dir = opendir(full_path);
    res.res = 0;
    

    if (res.res_dir == NULL)
        res.res = -errno;

    printf("OPENDIR path %s  result  %d\n", full_path, res.res);

    send(cfd, &res, sizeof(struct opendir_result), 0);
}

void s_readdir(struct client_response *info)
{
    struct readdir_result res;

    res.res = 0;
    struct dirent *de;

    de = readdir(info->sent_dir);
    if (de == 0)
        res.res = -errno;

    res.buff[0] = '\0';

    printf("%s\n\n\n\n","");
    for (de; de != NULL; de = readdir(info->sent_dir))
    {
        printf("%s\n", de->d_name);
        strcat(res.buff, de->d_name);
        strcat(res.buff, "/");
    }
    printf("READDDIIIRRR   %s\n", res.buff);

    send(cfd, &res, sizeof(struct readdir_result), 0);
}

void s_open(struct client_response *info)
{
    int res;

    char full_path[512];
    memcpy(&full_path, server_path, strlen(server_path) + 1);
    memcpy(&full_path[strlen(server_path)], info->string1, strlen(info->string1) + 1);

    res = open(full_path, info->int1);

    if (res < 0)
        res = -errno;
    
    send(cfd, &res, sizeof(int), 0);
}

void s_read(struct client_response *info)
{
    char buf[info->size];

    char full_path[512];
    memcpy(&full_path, server_path, strlen(server_path) + 1);
    memcpy(&full_path[strlen(server_path)], info->string1, strlen(info->string1) + 1);

    pread(info->int1, buf, info->size, info->off);
    send(cfd, buf, info->size, 0);
}

void s_rename(struct client_response *info)
{
    int res;

    char full_path[512];
    memcpy(&full_path, server_path, strlen(server_path) + 1);
    memcpy(&full_path[strlen(server_path)], info->string1, strlen(info->string1) + 1);

    char new_full_path[512];
    memcpy(&new_full_path, server_path, strlen(server_path) + 1);
    memcpy(&new_full_path[strlen(server_path)], info->string2, strlen(info->string2) + 1);

    res = rename(full_path, new_full_path);
    if (res < 0)
        res = -errno;

    send(cfd, &res, sizeof(int), 0);
}

void s_unlink(struct client_response *info)
{
    int res;

    char full_path[512];
    memcpy(&full_path, server_path, strlen(server_path) + 1);
    memcpy(&full_path[strlen(server_path)], info->string1, strlen(info->string1) + 1);

    res = unlink(full_path);
    if (res < 0)
        res = -errno;

    int write_res = send(cfd, &res, sizeof(int), 0);
}

void s_rmdir(struct client_response *info)
{
    int res;

    char full_path[512];
    memcpy(&full_path, server_path, strlen(server_path) + 1);
    memcpy(&full_path[strlen(server_path)], info->string1, strlen(info->string1) + 1);

    res = rmdir(full_path);
    if (res < 0)
        res = -errno;

    send(cfd, &res, sizeof(int), 0);
}

void s_truncate(struct client_response *info)
{
    int res;

    char full_path[512];
    memcpy(&full_path, server_path, strlen(server_path) + 1);
    memcpy(&full_path[strlen(server_path)], info->string1, strlen(info->string1) + 1);

    res = truncate(full_path, info->size);

    if (res < 0)
        res = -errno;

    send(cfd, &res, sizeof(int), 0);
}

void s_release(struct client_response *info)
{
    int res;
    res = close((uintptr_t)info->sent_dir);
    if (res < 0)
        res = -errno;

    send(cfd, &res, sizeof(int), 0);
}

void s_releasedir(struct client_response *info)
{
    int res;
    res = closedir((DIR *)(uintptr_t)info->sent_dir);
    if (res < 0)
        res = -errno;

    send(cfd, &res, sizeof(int), 0);
}

void s_write(struct client_response *info)
{
    int res;

    char full_path[512];
    memcpy(&full_path, server_path, strlen(server_path) + 1);
    memcpy(&full_path[strlen(server_path)], info->string1, strlen(info->string1) + 1);
    
    char buf[info->size];
    res = recv (cfd, buf, info->size, 0);  

    res = pwrite((uintptr_t)  info->sent_dir, buf, info->size, info->off);
    if(res < 0)
        res = -errno;

    send(cfd, &res, sizeof(int), 0);
}

int main(int argc, char *argv[])
{
    char *ip = strtok(argv[1], ":");
    int port = atoi(strtok(NULL, ":"));
    char *dir = argv[2];

    struct sockaddr_in addr;
    struct sockaddr_in peer_addr;
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(sfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char *)&optval, sizeof(optval));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    listen(sfd, BACKLOG);
    int peer_addr_size = sizeof(struct sockaddr_in);

    cfd = accept(sfd, (struct sockaddr *)&peer_addr, &peer_addr_size);
    printf("%s\n", "Connected ---------------------------------   ");
    while (1)
    {
        struct client_response *info = malloc(sizeof(struct client_response));
        recv(cfd, info, sizeof(struct client_response), 0);
        if (info->func == GETATTR)
            s_getattr(info);
        if (info->func == MKNOD)
            s_mknod(info);
        if (info->func == MKDIR)
            s_mkdir(info);
        if (info->func == OPENDIR)
            s_opendir(info);
        if (info->func == READDIR)
            s_readdir(info);
        if (info->func == RENAME)
            s_rename(info);
        if (info->func == UNLINK)
            s_unlink(info);
        if (info->func == RMDIR)
            s_rmdir(info);
        if (info->func == TRUNCATE)
            s_truncate(info);
        if (info->func == OPEN)
            s_open(info);
        if (info->func == READ)
            s_read(info);
        if (info->func == RELEASE)
            s_release(info);
        if (info->func == RELEASEDIR)
            s_releasedir(info);
        if (info->func == WRITE)
            s_write(info);
        free(info);
    }

    close(cfd);
    close(sfd);
    return 0;
}
