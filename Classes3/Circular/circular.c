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

MODULE_LICENSE("GPL");

#define CIRCULAR_MINOR 200

int buff_size = 10;
int curr_position = 0;
char* buffer;
static struct miscdevice device;

const struct file_operations device_fops;
const struct proc_ops proc_fops;
struct proc_dir_entry* proc_entry;

const char* device_name = "circular";

static int __init circular_init(void){
    proc_entry = proc_create("circular", 0000, NULL, &proc_fops);
	if (!proc_entry) {
		printk(KERN_WARNING "Cannot create /proc/simple\n");
        return 0;
    }
    device.minor = CIRCULAR_MINOR;
    device.name = device_name;
    device.fops = &device_fops;
    int result = misc_register(&device);
    if(result < 0){
        printk(KERN_WARNING, "Unable to register circular device");
    }
    buffer = kmalloc(buff_size, GFP_KERNEL);
    if(!buffer){
        result = -ENOMEM;
        misc_deregister(&device);
        return result;
    }
    int i;
    for(i = 0; i<buff_size; i++){
        buffer[i] = '\0';
    }
    return result;
}

static void __exit circular_exit(void){
    misc_deregister(&device);
    kfree(buffer);
    if (proc_entry) {
		proc_remove(proc_entry);
	}
    printk(KERN_INFO, "Unregistered circular device");
}

ssize_t circular_write(struct file *filp, const char __user *user_buf, size_t count, loff_t *f_pos){
    int i;
    for(i = 0; i<count; i++){
        if(copy_from_user(buffer + curr_position, user_buf + i, 1)){
            printk(KERN_WARNING, "unable to copy char from user\n");
            return -EFAULT;
        }
        curr_position += 1;
        curr_position %= (buff_size - 1);
    }
    return count;
}

ssize_t circular_read(struct file *filp, char __user *user_buf, size_t count, loff_t *f_pos){
    size_t to_copy = strlen(buffer);

    if(*f_pos >= to_copy){
        return 0;
    }

    if(copy_to_user(user_buf, buffer, to_copy)){
        printk(KERN_WARNING, "unable to copy data to user\n");
        return -EFAULT;
    }
    *f_pos += to_copy;
    return to_copy;
}

ssize_t circular_proc_write(struct file *filp, const char __user *user_buf, size_t count, loff_t *f_pos){
    char* temp_buf = kmalloc(count + 1, GFP_KERNEL);
    long value;
    if(!temp_buf){
        printk("unable to allocate temporary buffer\n");
        return -ENOMEM;
    }
    if(copy_from_user(temp_buf, user_buf, count)){
        printk("unable to copy from user buff\n");
        return -EFAULT;
    }
    temp_buf[count] = '\0';
    int parse_code = kstrtol(temp_buf, 10, &value);
    if(parse_code){
        printk("error converting string input to integer, error is: %d\n", parse_code);
        return -EFAULT;
    }
    kfree(temp_buf);
    char* new_buffer = kmalloc(value, GFP_KERNEL);
    if(!new_buffer){
        printk("Unable to allocate new buffer\n");
        return -ENOMEM;
    }
    int i;
    for(i = 0; i<value && i<buff_size; i++){
        new_buffer[i] = buffer[i];
    }
    buff_size = value;
    kfree(buffer);
    buffer = new_buffer;
    curr_position %= (buff_size - 1);
    buffer[buff_size - 1] = '\0';
    return count;
}

const struct file_operations device_fops = {
    .read = circular_read,
    .write = circular_write,
};

const struct proc_ops proc_fops = {
	.proc_write = circular_proc_write,
};

module_init(circular_init);
module_exit(circular_exit);