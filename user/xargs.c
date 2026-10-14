#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

char *args[] = {"echo", "hello", 0};

int
main(int argc, char *argv[])
{
    int pid = fork(), status;
    if (pid == 0) {
        // Child
        exec(args[0], args);
        // Should not reach here if exec is successful
        fprintf(2, "xargs: exec failed\n");
        exit(1);
    } else if (pid > 0) {
        // Parent
        wait(&status);
        printf("status = %d\n", status);
    } else {
        // Error
        fprintf(2, "xargs: fork failed\n");
        exit(1);
    }
    exit(0);
}