#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched/signal.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/sched/task.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/slab.h>

#define MAX_UID 32767

SYSCALL_DEFINE1(timeuser, uid_t*, usr_id){
    struct task_struct* proc;

    u64* per_user_cpu_time = (u64*) kmalloc(sizeof(u64) * MAX_UID, GFP_KERNEL);

    if(!per_user_cpu_time){
        printk(KERN_WARNING, "Unable to allocate array for every user\n");
        return -ENOMEM;
    }

    int i;
    uid_t max_user;
    u64 max_cpu = 0;

    for(i = 0; i<MAX_UID; i++){
        per_user_cpu_time[i] = 0;
    }

    read_lock(&tasklist_lock);
    for_each_process(proc){
        int uid = proc->cred->uid.val;
        per_user_cpu_time[uid] += proc->utime;

        if(per_user_cpu_time[uid] > max_cpu){
            max_cpu = per_user_cpu_time[uid];
            max_user = uid;
        }
    }
    read_unlock(&tasklist_lock);
    
    kfree(per_user_cpu_time);

    *usr_id = max_user;

    return 0;
}
