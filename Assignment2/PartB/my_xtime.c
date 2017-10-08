#include <linux/linkage.h>
#include <linux/export.h>
#include <linux/time.h>
#include <asm/uaccess.h>
#include <linux/printk.h>
#include <linux/slab.h>
asmlinkage int my_xtime(struct timespec *current_time){
    
    if (access_ok(VERIFY_WRITE, current_time, sizeof(current_time))) {
        
        *current_time = current_kernel_time();
        printk(KERN_ALERT "current_time: %ld!\n",current_time->tv_nsec);
        return EFAULT;
    }
    
    return 0;
}
EXPORT_SYMBOL(my_xtime);
