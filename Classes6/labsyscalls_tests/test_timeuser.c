#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <errno.h>

#define timeuser_syscall 441

int main() {
    uid_t max_user;
    long result = syscall(timeuser_syscall, &max_user);
    printf("Results: %d %d\n", result, max_user);

    return 0;
}