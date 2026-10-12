#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

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

// Do ls but recursive
void
find(char *path)
{

}

// Parse args and call find
int
main(int argc, char *argv[])
{

}