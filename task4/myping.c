
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h> // gettimeofday()
#include <sys/types.h>
#include <unistd.h>

int count = 0;

void killhandler(int sig){
    printf("SIGKILL received\n");
    return;
}

void childhandler(int sig){
    int status;
    wait(&status);
    count += WEXITSTATUS(status);
    return;
}

int main(){
    int i; // for loop iterator
    pid_t pid[3]; // pids of child processes
    Signal(SIGKILL, killhandler);
    Signal(SIGCHLD, childhandler);
    // Fork 3 child processes
    for(i=0; i<3; i++){
        pid[i] = fork();
        if(!pid[i]){ // If child process
            Signal(SIGKILL, SIG_DFL);
            exit(5);
        }
    }

    // Parent process only
    for(i=0; i<3; i++){
        kill(pid[i], SIGKILL);
    }
    sleep(5);
    printf("count = %d\n", count);

    exit(0);
}