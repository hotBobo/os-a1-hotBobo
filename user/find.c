#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"

// Check if pattern matches name, 1 if true
int
match(char *pattern, char *name)
{
    // If pattern empty, return true only if name also empty obv
    if(*pattern == '\0') return *name == '\0';
    // If pattern is a star, return true only if:
    // Pattern without * matches name (*a.c == a.c) or
    // Pattern with * matches name w/o first char (*a.c == sdsa.c) and name is not empty (so it can be matched)
    // Because it's recursive, chars will be taken off until name reaches end or pattern without star will reach name
    if (*pattern == '*') return match(pattern+1,name) || (*name != '\0' && match(pattern,name+1));
    // If all else fails, true only if name is not at end, (both will not be at end cause first cond)
    // CharPattern equals CharName
    // And both incremented still return matched
    return *name != '\0' && *pattern == *name && match(pattern+1,name+1);
}
// That sucked to figure out

// Do ls but recursive and with pattern matching and type checking
void
find(char *path, char *pattern, int type)
{
    char buf[MAXPATH], *p;
    int fd;
    struct dirent de;
    struct stat st;
    // Opens the file path
    if((fd = open(path, O_RDONLY)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        exit(1);
    }
    // Get the files stats, specifically need type (dir, file, etc)
    if(fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        exit(1);
    }
    // Make sure we are in a directory
    if(st.type != T_DIR) {
        close(fd);
        return;
    }
    // Check that path isn't longer than buf can handle
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
        fprintf(2, "find: path too long\n");
        close(fd);
        exit(1);
    }
    // Copy path into buf, set p to end of path in buf, add a slash for the next file
    strcpy(buf, path);
    p = buf + strlen(buf);
    if (p != buf && *(p-1) != '/')
        *p++ = '/'; // First add / then increment p so it points to the next char after the slash
    
    // sizeof(de) is fixed to 16, so will be true only if it reads a full dirent struct
    // Once done reading, the open located in fd is incremented by 16, so we can read the next one
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
        // If 0 then slot empty, so just skip
        if (de.inum == 0) continue;
        // Concat name to p and set end to '\0'
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if (strcmp(p, ".") == 0 || strcmp(p, "..") == 0) continue; // Skip . and .. directories
        // Buf now has path to subdirectory/file, so get stats for it
        if (stat(buf, &st) < 0) {
            fprintf(2, "find: cannot stat %s\n", buf);
            continue;
        }
        // If the name/type matches the name/type we are looking for, print it
        if ((type == 0 || type ==st.type || (type == T_FILE && st.type == T_DEVICE)) && match(pattern, p)) {
            printf("%s\n", buf);
        }
        // If type is directory, recursively call find on it
        if (st.type == T_DIR) {
            find(buf, pattern, type);
        }
    }
    // Close the file descriptor for the directory we opened at the start of the function so that when we recurse, we aren't opening too many files at once and running out of file descriptors
    // This makes it so that we can be 13 directories deep, not just have 13 directories
    close(fd);
}

// Parse args and call find
int
main(int argc, char *argv[])
{
    // If there are 5 args, check that the 4th is -type and the 5th is f or d, then call find with the correct type
    if (argc == 5) {
        if (strcmp(argv[3], "-type") != 0) {
            fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
            exit(1);
        }
        if (strcmp(argv[4], "f") == 0) {
            find(argv[1], argv[2], T_FILE);
        } else if (strcmp(argv[4], "d") == 0) {
            find(argv[1], argv[2], T_DIR);
        } else {
            fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
            exit(1);
        }
    } else if (argc == 3) {
        find(argv[1], argv[2], 0); // 0 means any type
    } else {
        fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
        exit(1);
    }
    exit(0);
}