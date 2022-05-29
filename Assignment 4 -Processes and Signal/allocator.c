#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        char *ptr;
        long mbs = strtol(argv[i], &ptr, 10);
        if (*ptr != '\0') {
            printf("%s is not a number!\n", argv[i]);
            continue;
        }
        printf("allocating %ldM\n", mbs);
        fflush(stdout);
        ptr = malloc(1024*1024*mbs);
        if (!ptr) printf("got NULL\n");
        fflush(stdout);
        strcpy(ptr, "ben was here");
    }
    exit(0);
}
