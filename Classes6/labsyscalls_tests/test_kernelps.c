#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#define kernelps_syscall 442

int main(){
    size_t proc_count;

    int expected_proc_count = 500;
    int max_comm_size = 256;

    char* procs[expected_proc_count + 1];

    for(int i = 0; i<expected_proc_count; i++){
        procs[i] = (char*) malloc(sizeof(char) * max_comm_size);
    }

    procs[expected_proc_count] = NULL;

    long result = syscall(kernelps_syscall, &proc_count, procs);

    if(result){
        printf("Call to kernelps unsuccesfull, see dmesg for details\n");
    }
    else{
        for(int i = 0; i<proc_count; i++){
            printf("%s\n", procs[i]);
        }
    }

    for(int i = 0; i<expected_proc_count; i++){
        free(procs[i]);
    }

    return 0;
}
