// Grade:100/100
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc,char **argv) {
    if (argc==1 || argc>2){
        printf("USAGE: duplicator count\ncount must be an integer greater than 0\n");
        exit(1);
    }
    else {
        char *line = NULL;
        char *ptr = NULL;
        int count = strtol(argv[1], &ptr, 10);
        if (count <= 0 || *ptr!='\0') {
            printf("USAGE: duplicator count\ncount must be an integer greater than 0\n");
            free(line);
            exit(1);
        }
        else {
            size_t len = 0;
            while (getline(&line, &len, stdin)!=-1) {
                line[strlen(line) - 1] = '\0';
                if (count <= 100) {
                    for (int i = 0; i < count; i++) {
                        printf("%s\n", line);
                    }
                } else {
                    printf("%s %d times\n", line, count);
                }
            }
        }
        free(line);
        exit(0);
    }
}
