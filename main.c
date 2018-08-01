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


int main() {
    FILE * f = fopen("STORAGE", "w");

    fwrite("\n" , 1 , 1 , f);
    fwrite("123123s\n" , 1 , 8 , f);
    fwrite("123123s\n" , 1 , 8 , f);
    fwrite("123123s\n" , 1 , 8 , f);
    fwrite("123123s\n" , 1 , 8 , f);

    fclose(f);
    printf("BABLABLA %d\n", 1);
}