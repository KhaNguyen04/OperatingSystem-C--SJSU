#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

pid_t child_id;

void alarm_handler(int sig){
    kill(child_id,SIGKILL);
}

int main(int argc,char** argv) {
    if (argc < 4) {
        printf("USAGE: ./limiter timelimit_secs memlimit_mb program args...\n");
        exit(1);
    } else {

        char *ptr1 =NULL;
        char *ptr2=NULL;
        int status;
        int time_limit=strtol(argv[1],&ptr1,10);
        int mem_limit=strtol(argv[2],&ptr2,10);
        if (*ptr1!='\0' || *ptr2!='\0'){
            printf("USAGE: ./limiter timelimit_secs memlimit_mb program args...\n");
            exit(1);
        }
        else {
//          Joel (DarthBodyPart) on discord shows me how to use setrlimit
            struct rlimit limit;
            limit.rlim_cur=mem_limit*1024*1024;
            limit.rlim_max=mem_limit*1024*1024;
            int fds[2];
            pipe(fds);
            int rc = fork();
            if (rc < 0) {
                printf("Fork fail");
                exit(1);
            }
            else if (rc==0){
                close(fds[0]);
                int id=getpid();
                char *myargs[argc-2];
                int exec;
                myargs[0]=strdup(argv[3]);
                write(fds[1],&id, sizeof id);
                close(fds[1]);
                if (argc>4){
                    int j=1;
                    for (int i=4; i<argc;i++){
                        myargs[j]= strdup(argv[i]);
                        j++;
                    }
                }
                myargs[argc-3]=NULL;
                setrlimit(RLIMIT_AS,&limit);
                exec=execvp(myargs[0],myargs);
                if (exec==-1){
                    perror(argv[3]);
                    exit(99);
                }
            }
            else{
                close(fds[1]);
                int childId;
                read(fds[0],&childId,sizeof childId);
                child_id=childId;
                signal(SIGALRM,alarm_handler);
                alarm(time_limit);
                child_id=wait(&status);
                if (WIFSIGNALED(status)){
                    int sig=WTERMSIG(status);
                    printf("killed due to %s\n", strsignal(sig));
                    exit(100+sig);
                }
                exit(WEXITSTATUS(status));

             }
        }
    }
    return 0;
}