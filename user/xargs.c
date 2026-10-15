#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

void
usageExit()
{
    fprintf(2, "usage: xargs [-n N] cmd [args...]\n");
    exit(1);
}

int
run(char **args)
{
    int status, pid = fork();
    // It's forkin' time
    if (pid == 0) {
        // Child
        // First arg is command, rest are args, last is null terminator
        exec(args[0], args);
        // Should not reach here if exec is successful
        fprintf(2, "xargs: exec failed\n");
        exit(1);
    } else if (pid > 0) {
        // Parent
        wait(&status);
        // printf("status = %d\n", status);
        return status;
    } else {
        // Error
        fprintf(2, "xargs: fork failed\n");
        exit(1);
    }
}

int
main(int argc, char *argv[])
{
    // Make sure there are actual arguments and stuff
    if (argc < 2) {
        usageExit();
    }
    // Create array and vars for -n
    char *args[MAXARG];
    int N = 0, cmd = 1;
    // If first arg is -n
    if (strcmp(argv[1], "-n") == 0) {
        // -n must have N and cmd
        cmd = 3;
        if (argc <= cmd) {
            usageExit();
        }
        // Make sure N is number
        for(int i = 0; argv[2][i] != '\0'; ++i) {
            if (argv[2][i] > '9' || argv[2][i] < '0') {
                usageExit();
            }
        } // Make sure N is not empty
        if (argv[2][0] == '\0') usageExit();
        N = atoi(argv[2]);
        if (N <= 0) usageExit();
    }
    // Error if too many args
    // Base is number of args including cmd, so if base + 1 (for null terminator) is greater than MAXARG, exit with error
    int base = argc - cmd;
    if (MAXARG < base+1) usageExit();
    

    // Fill args[] with arguments after xargs
    for(int i=0; i<base; ++i) {
        args[i] = argv[i+cmd];
    }
    args[base] = 0; // Null terminate args[]

    // New is 1 if we are at the start of a new argument, pos is position in buf, nwords is number of words read, failed is 1 if any run() call failed
    char buf[512], c;
    int new = 1, pos = 0, nwords = 0, failed = 0;
    // Read from stdin and fill buf with arguments, then call run() when we have N arguments or when we reach the end of input
    // Read one character in at a time, 0 makes it from stdin
    while (read(0, &c, 1) == 1) {
        if(c == ' ' || c == '\t' || c == '\n') {
            // If space and not new, then we have a new argument, so add it to args[] and increment nwords
            if (!new) {
                ++nwords;
                buf[pos++] = '\0';
                // If we have N arguments, call run() and reset nwords and pos to overwrite buf
                if (N > 0 && nwords == N) {
                    args[base+nwords] = 0; // Make sure we have terminator
                    // If it returned non-zero then set failed to 1
                    if(run(args) != 0) failed = 1;
                    nwords = 0; pos = 0;
                }
                new = 1;
                // Make sure we don't exceed MAXARG, if so exit with error
                if(base + nwords >= MAXARG) {
                    fprintf(2, "xargs: too many arguments\n");
                    exit(1);
                }
            }
            // If endl and no N restrictions, ruuuunnnnnnn
            if (c == '\n' && N == 0 && nwords > 0) {
                args[base+nwords] = 0; // Make sure we have terminator
                // If it returned non-zero then set failed to 1
                if(run(args) != 0) failed = 1;
                nwords = 0; pos = 0;
            }
        } else {
            // If not space and new, then we have a new argument, so add pointer to location in buf to args[] and increment nwords
            if (new) {
                args[base+nwords] = &buf[pos];
                new = 0;
            }
            // Make sure we don't exceed buf size with \0, if so exit with error
            if (pos >= sizeof(buf)-1) {
                fprintf(2, "xargs: input too long\n");
                exit(1);
            }
            // Add character to buf
            buf[pos++] = c;
        }
    }
    // If we are not at the start of a new argument, then we have a new argument, so add it to args[] and increment nwords
    if (!new) {buf[pos++] = '\0'; ++nwords;}
    if (nwords > 0) {args[base+nwords]=0; if(run(args) != 0) failed = 1;}
    // If it returned 0 then end of input
    exit(failed);
}