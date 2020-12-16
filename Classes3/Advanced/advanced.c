#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

#define PRNAME_MINOR 199
#define JIFFIES_MINOR 198

MODULE_LICENSE("GPL");

static struct miscdevice prname_device;
const struct file_operations prname_fops;
long proc_id = -1;
bool prname_written = false;

static struct miscdevice jiffies_device;
const struct file_operations jiffies_fops;

ssize_t prname_write(struct file *filp, const char __user *user_buf, size_t count, loff_t *f_pos){
    char* temp_buffer = kmalloc(count + 1, GFP_KERNEL);
    temp_buffer[count] = '\0';
    if(!temp_buffer){
        printk("unable to allocate temporary buffer\n");
        return -ENOMEM;
    }
    if(copy_from_user(temp_buffer, user_buf, count)){
        printk("unable to copy from user\n");
        return -EFAULT;
    }
    if(kstrtol(temp_buffer, 10, &proc_id)){
        printk("error parsing user input\n");
        return -EINVAL;
    }
    if(!find_get_pid((int) proc_id)){
	prname_written = false;
	printk("proccess id is not valid\n");
	return -EINVAL;
    }
    prname_written = true;
    return count;
}
ssize_t prname_read(struct file *filp, char __user *user_buf, size_t count, loff_t *f_pos){
    if(!prname_written){
	printk("no valid entry was done\n");
	return -EFAULT;
    }
    struct task_struct* taskp = get_pid_task(find_get_pid((int) proc_id), PIDTYPE_PID);
    if(!taskp){
	printk("task not found\n");
	return 0;
    }
    char* prname_buffer = kmalloc(strlen(taskp->comm) + 1, GFP_KERNEL);
    if(!prname_buffer){
	printk("unable to allocate buffer for proc name\n");
	return -ENOMEM;
    }
    prname_buffer[strlen(taskp->comm)] = '\0';
    strcpy(prname_buffer, (char*) taskp->comm);
    int to_copy = strlen(prname_buffer);
    if(*f_pos >= to_copy){
	return 0;
    }
    if(copy_to_user(user_buf, prname_buffer, to_copy)){
	printk("Unable to send task name to user\n");
	kfree(prname_buffer);
	return -EFAULT;
    }
    *f_pos += to_copy;
    kfree(prname_buffer);
    return to_copy;
}

ssize_t jiffies_read(struct file *filp, char __user *user_buf, size_t count, loff_t *f_pos){
    unsigned long curr_jiffies = jiffies;
    char* buff = kmalloc(50, GFP_KERNEL);
    if(!buff){
        printk("unable to allocate buffer for jiffies\n");
        return -ENOMEM;
    }
    sprintf(buff, "%ld", curr_jiffies);
    int to_copy  = strlen(buff);

    if(*f_pos >= to_copy){
        kfree(buff);
        return 0;
    }

    if(copy_to_user(user_buf, buff, to_copy)){
        printk("unalbe to send jiffies to user\n");
        kfree(buff);
        return -EFAULT;
    }

    *f_pos += to_copy;
    kfree(buff);
    return to_copy;
}

static int __init advanced_init(void){
    prname_device.minor = PRNAME_MINOR;
    prname_device.name = "prname";
    prname_device.fops = &prname_fops;

    jiffies_device.minor = JIFFIES_MINOR;
    jiffies_device.name = "jiffies";
    jiffies_device.fops = &jiffies_fops;

    if(misc_register(&prname_device) < 0){
        printk("unable to register /dev/prname\n");
        return -EFAULT;
    }
    if(misc_register(&jiffies_device) < 0){
        printk("unable to register /dev/jiffies\n");
        return -EFAULT;
    }
    return 0;
}

static void __exit advanced_exit(void){
    misc_deregister(&prname_device);
    misc_deregister(&jiffies_device);
}

const struct file_operations prname_fops = {
    .write = prname_write,
    .read =  prname_read,
};

const struct file_operations jiffies_fops = {
    .read = jiffies_read,
};

module_init(advanced_init);
module_exit(advanced_exit);

