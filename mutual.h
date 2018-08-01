#ifndef UTILS_H
#define UTILS_H

#include <sys/stat.h>
#include <dirent.h>
#include "map.h"

#define GETATTR 1
#define MKNOD 2
#define MKDIR 3
#define OPENDIR 4
#define READDIR 5
#define OPEN 6
#define READ 7
#define RENAME 8
#define UNLINK 9
#define RMDIR 10
#define TRUNCATE 11
#define RELEASE 12
#define RELEASEDIR 13
#define WRITE 14

struct client_response
{
	int func;
	char string1[256];
	char string2[256];
	int int1;

	size_t size;
	off_t off;

	DIR * sent_dir;
};

struct getattr_result
{
	int res;
	struct stat buf;
};

struct opendir_result
{
	int res;
	DIR *res_dir;
};

struct readdir_result
{
	int res;
	char buff[8192];
};

struct mutual_server {
	int sfd1;
	int sfd2;

	struct map mp;
	FILE * fd;
	
	char server1 [64];
	char server2 [64];
};

#endif