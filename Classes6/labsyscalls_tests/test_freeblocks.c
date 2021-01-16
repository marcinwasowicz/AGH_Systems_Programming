#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>


#define freeblocks_syscall 443

int main(int argc, char* argv[]){
    char* file_path = argv[1];
    u_int64_t freeblocks_count;

    long result = syscall(freeblocks_syscall, file_path, &freeblocks_count);

    if(result){
        printf("Call unsuccesfull, see dmesg for details\n");
    }
    else{
        printf("Free blocks count obtained for given path is: %lld\n", freeblocks_count);
    }

    return 0;
}