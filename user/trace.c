#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
usageExit()
{
    fprintf(2, "usage: trace <mask> <cmd> [args...]\n");
    exit(1);
}

int
main(int argc, char *argv[])
{
    // Make sure we have at least 3 arguments: the program name, the trace mask, and the command to execute
    if (argc < 3) {
        usageExit();
    }
    // Make sure the trace mask is a valid integer
    int i;
    for(i = 0; argv[1][i] != '\0'; ++i) {
        if (argv[1][i] > '9' || argv[1][i] < '0') {
            usageExit();
        }
    } if (i == 0) usageExit();
    // Set trace mask, if returns negative, print error message and exit (this should never happen)
    if (trace(atoi(argv[1])) < 0) {
        fprintf(2, "trace: failed to set trace mask\n");
        exit(1);
    }
    // Execute the command with the provided arguments, if it returns, print error message and exit
    exec(argv[2], argv + 2);
    fprintf(2, "trace: failed to exec %s\n", argv[2]);
    exit(1);
}