#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    if (argc != 2) {fprintf(2,"usage: sleep <ticks>\n"); exit(1);}
    int i, n;
    for(i = 0; argv[1][i] != '\0'; ++i) {
        if (argv[1][i] > '9' || argv[1][i] < '0') {
            fprintf(2,"usage: sleep <ticks>\n");
            exit(1);
        }
    }
    if (i == 0) {fprintf(2,"usage: sleep <ticks>\n"); exit(1);}
    n = atoi(argv[1]);
    pause(n);
    exit(0);
}