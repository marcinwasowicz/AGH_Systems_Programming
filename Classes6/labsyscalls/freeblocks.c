#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/namei.h>
#include <linux/mount.h>
#include <linux/fs.h>
#include <linux/statfs.h>

SYSCALL_DEFINE2(freeblocks, char*, path, u64*, count){
    struct path fs_path;
    struct kstatfs fs_metadata;

    if(kern_path(path, LOOKUP_FOLLOW, &fs_path)){
        printk("Unable to obtain path struct\n");
        return -EFAULT;
    }

    if(vfs_statfs(&fs_path, &fs_metadata)){
        printk("Unable to obtain kstatfs struct\n");
        return -EFAULT;
    }

    *count = fs_metadata.f_bfree;

    return 0;
}