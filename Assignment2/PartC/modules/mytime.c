#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/time.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

MODULE_LICENSE("DUAL BSD/GPL");

static int device_open(struct inode *inode, struct file *file)
{
    static int counter = 0;
    if (Device_Open) return -EBUSY;
    
    Device_Open++;
    printk(KERN_ALERT "Open times: %d!\n", counter++);
    msg_Ptr = msg;
    MOD_INC_USE_COUNT;
    
    return SUCCESS;
}
static int my_read(struct file *file, char __user *out, size_t len, loff_t *ppos)
{
    int err = -EFAULT;
    if (access_ok(VERIFY_WRITE, current_time, len)) {
        struct timespec time = current_kernel_time();
        struct timespec time_day;
        getnstimeofday(&time_day);
        
    
        char *buf = kmalloc(200,GFP_KERNEL);
        sprintf(buf, "current_kernel_time: %.9ld %.6ld\ngetnstimeofday: %.9ld %.6ld\n",time.tv_sec,time.tv_nsec,time_day.tv_sec,time_day.tv_nsec);
        printk(KERN_ALERT "%s",buf);
        err =  copy_to_user(out, buf, strlen(buf)+1);
        if (err != 0) {
            printk(KERN_ALERT "Copy Error:%d",err);
        }
        free(buf);
    }
    
    
    return err;
}
static ssize_t my_write(struct file *file, const char __user *buf,
                            size_t len, loff_t *ppos)
{
    printk(KERN_ALERT "Yummy - I just ate %d bytes\n", len);
    return len;
}

static int my_close(struct inode *inodep, struct file *file)
{
    printk(KERN_ALERT "Sleepy time\n");
    return 0;
}


static struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
    .read = my_read,
    .write = my_write
};


static struct miscdevice my_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "mytime",
    .fops = &my_fops
};



// called when module is installed
int __init mytime_init()
{
    misc_register(&my_misc_device);
    printk(KERN_ALERT "mytime init!\n");
    
    return 0;
}



// called when module is removed
void __exit mytime_exit()
{
    misc_deregister(&my_misc_device);
    printk(KERN_ALERT "mytime exit!!\n");
}

module_init(mytime_init);
module_exit(mytime_exit);
