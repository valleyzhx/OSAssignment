#include <linux/linkage.h>
#include <linux/export.h>
#include <linux/time.h>
#include <asm/uaccess.h>
#include <linux/printk.h>
#include <linux/slab.h>

asmlinkage int sys_my_xtime(struct timespec *current_time){
    
    if (access_ok(VERIFY_WRITE, current_time, sizeof(current_time))) {
        struct timespec  time = current_kernel_time();
        copy_to_user(current_time,&time,sizeof(current_time));
        printk(KERN_ALERT "current_time: %ld!\n",current_time->tv_nsec);
        return 0;
    }
    return -EFAULT;

    
}
EXPORT_SYMBOL(sys_my_xtime);
