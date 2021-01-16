#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#define pidtoname_syscall 444

int main(int argc, char* argv[]){
    pid_t proc_id = atoi(argv[1]);
    int expected_name_size = atoi(argv[2]);

    char* proc_name = (char*) malloc(sizeof(char) * expected_name_size);
    long result = syscall(pidtoname_syscall, proc_id, proc_name);

    if(result){
        printf("Call unsuccesfull, see dmesg for details\n");
    }
    else{
        printf("Process of id: %d has name: %s\n", proc_id, proc_name);
    }

    free(proc_name);

    return 0;
}