#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched/signal.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/sched/task.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/slab.h>

SYSCALL_DEFINE2(kernelps, size_t*, ps_count, char**, buff){
    *ps_count = 0;
    int iter = 0;
    struct task_struct* proc;

    read_lock(&tasklist_lock);
    for_each_process(proc){
        (*ps_count)++;
        if(buff == NULL){
            continue;
        }

        if(buff[iter] == NULL){
            read_unlock(&tasklist_lock);
            printk(KERN_INFO, "User array was not large enough to store all processes\n");
            return -EFAULT;
        }

        if(copy_to_user(buff[iter], proc->comm, strlen(proc->comm))){
            read_unlock(&tasklist_lock);
            printk(KERN_WARNING, "Could not copy to user\n");
            return -EFAULT;
        }
        iter++;
    }
    read_unlock(&tasklist_lock);

    return 0;
}