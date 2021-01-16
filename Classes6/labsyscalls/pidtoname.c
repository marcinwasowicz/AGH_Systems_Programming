#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/sched/task.h>
#include <linux/proc_fs.h>

SYSCALL_DEFINE2(pidtoname, pid_t , proc_id, char*,  proc_name){
    struct task_struct* proc = get_pid_task(find_get_pid((int) proc_id), PIDTYPE_PID);

    if(!proc){
        printk("Unable to resolve process based if pid\n");
        return -EFAULT;
    }

    if(copy_to_user(proc_name, proc->comm, strlen(proc->comm))){
        printk("Unalbe to copy process name to user. Buffer may not be large enough\n");
        return -EFAULT;
    }

    return 0;
}