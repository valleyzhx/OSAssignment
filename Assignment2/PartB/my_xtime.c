#include <linux/linkage.h>
#include <linux/export.h>
#include <linux/time.h>
#include <asm/uaccess.h>
#include <linux/printk.h>
#include <linux/slab.h>

asmlinkage int sys_my_xtime(struct timespec *current_time){
    
    if (access_ok(VERIFY_WRITE, current_time, sizeof(struct timespec))) {
        struct timespec  time = current_kernel_time();
        int error =  copy_to_user(current_time,&time,sizeof(struct timespec));
        if (error != 0) {
            printk(KERN_ALERT "Copy Error:%d",error);
        }
        printk(KERN_ALERT "current_time in system: %.9ld!\n",time.tv_nsec);
        return error;
    }
    return -EFAULT;

    
}
EXPORT_SYMBOL(sys_my_xtime);
