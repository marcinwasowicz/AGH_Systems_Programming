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

SYSCALL_DEFINE1(topuser, uid_t*, usr_id){
    struct task_struct* proc;

    uid_t* proc_per_usr_cnt = (uid_t*) kmalloc(MAX_UID * sizeof(uid_t), GFP_KERNEL);

    if(!proc_per_usr_cnt){
        printk(KERN_WARNING, "unable to allocate array for each user\n");
        return -ENOMEM;
    }
    int i;
    for(i = 0; i<MAX_UID; i++){
        proc_per_usr_cnt[i] = 0;
    }

    int max_count = 0;
    uid_t max_usr = 0;
    
    read_lock(&tasklist_lock);
    for_each_process(proc){
        proc_per_usr_cnt[proc->cred->uid]++
        if(proc_per_usr_cnt[proc->cred->uid] > max_count){
            max_count = proc_per_usr_cnt[proc->cred->uid];
            max_usr = proc->cred->uid
        }
    }
    read_unlock(&tasklist_lock);

    kfree(proc_per_usr_cnt);

    *usr_id = max_usr;

    return 0;
}
