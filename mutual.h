#ifndef UTILS_H
#define UTILS_H

#include <sys/stat.h>
#include <dirent.h>

enum syscall
{
	GETATTR,
	MKNOD,
	MKDIR,
	OPENDIR,
	READDIR,
	OPEN,
	READ,
	RENAME,
	UNLINK,
	RMDIR,
	TRUNCATE,
	RELEASE,
	RELEASEDIR,
	WRITE
};

struct client_response
{
	enum syscall func;
	char string1[256];
	char string2[256];
	int int1;

	size_t size;
	off_t off;

	DIR *sent_dir;
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

#endif