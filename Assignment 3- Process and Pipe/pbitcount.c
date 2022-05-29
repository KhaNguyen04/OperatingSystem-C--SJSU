#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc,char** argv) {
    if (argc==1){
        printf("USAGE: ./pbitcount filenames\n");
        exit(1);
    }
    else{
        for (int i=1;i<argc;i++){
            int fd = open(argv[i], O_RDONLY);
            if (fd==-1){
                perror(argv[i]);
                close(fd);
                exit(2);
            }
            close(fd);
        }
        int total;
        for (int i=1;i<argc;i++){
            int fds[2];
            int rc_pipe=pipe(fds);
            if (rc_pipe==-1){
                perror("pipe");
                exit(2);
            }
            int rc_fork=fork();
            if (rc_fork<0){
                printf("Fork fail");
                exit(1);
            }
            else if (rc_fork==0){
                close(fds[0]);
                int fd = open(argv[i], O_RDONLY);
                if (fd==-1){
                    perror(argv[i]);
                    close(fd);
                    exit(2);
                }
                char buffer[128];
                int count = 0;
                ssize_t rc_read;
                while ((rc_read=read(fd, buffer, 128))!=0) {
                    for (int j=0;j<rc_read;j++) {
                        unsigned char var=buffer[j];
                        for (int k=0;k<8;k++) {
                            if (var!=0){
                                count+= var & 1;
                                var>>= 1;
                            }
                            else
                                break;
                        }
                    }
                }
                write(fds[1],&count,sizeof count);
                exit(0);
            }
            else{
                close(fds[1]);
                int p_count;
                read(fds[0],&p_count,sizeof p_count);
                total+=p_count;
            }
        }
        printf("%d\n",total);
    }
    return 0;
}
