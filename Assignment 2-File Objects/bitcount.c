#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


int main(int argc,char** argv) {
    if (argc==1 || argc>2){
        printf("USAGE: ./bitcount filename\n");
        exit(1);
    }
    else {
        int fd = open(argv[1], O_RDONLY);
        if (fd==-1){
            perror(argv[1]);
            close(fd);
            exit(2);
        }
        char buffer[128];
        int count = 0;
        ssize_t rc;
        while ((rc=read(fd, buffer, 128))!=0) {
//          Good To Sea on discord explains that looping through the buffer may have problem on the last iteration so
//          using rc instead
            for (int i=0;i<rc;i++) {
//             Joel (DarthBodyPart on Discord) helps me to realize the
                unsigned char var=buffer[i];
                for (int j=0;j<8;j++) {
                    if (var!=0){
//            https://www.geeksforgeeks.org/count-set-bits-in-an-integer/
//            to count bit, check with bitwise AND, then left shift to the next, until all value is zero
                        count+= var & 1;
                        var>>= 1;
                    }
                    else
                        break;

                }
            }
        }
        printf("%d\n", count);
        close(fd);
    }
}
//for (int i=0;ch[i]!='\0';i++) {

//for (int i=0;i<128;i++) {

