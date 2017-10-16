#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/time.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/printk.h>
#include <linux/slab.h>

#define SUCCESS 0


MODULE_LICENSE("DUAL BSD/GPL");

static int my_open(struct inode *inode, struct file *file);
static int my_close(struct inode *inodep, struct file *file);

static ssize_t my_read(struct file *file, char __user *out, size_t len, loff_t *ppos);
static ssize_t my_write(struct file *file, const char __user *buf,
                        size_t len, loff_t *ppos);


static int my_open(struct inode *inode, struct file *file)
{
    
    printk(KERN_ALERT "Open !\n");
    
    return SUCCESS;
}
static ssize_t my_read(struct file *file, char __user *out, size_t len, loff_t *ppos)
{
    int err = 0;
    if (access_ok(VERIFY_WRITE, out, len)) {
        struct timespec time = current_kernel_time();
        struct timespec time_day;
        getnstimeofday(&time_day);
        
    
        char buf[200];
        sprintf(buf, "current_kernel_time: %9ld %6ld\ngetnstimeofday: %9ld %6ld\n",time.tv_sec,time.tv_nsec,time_day.tv_sec,time_day.tv_nsec);
        printk(KERN_ALERT "%s\n",buf);
        err =  copy_to_user(out, buf, 200);
        //printk(KERN_ALERT "Str length:%ld\n",strlen(buf));

        if (err != 0) {
            printk(KERN_ALERT "Copy Error:%d\n",err);
            return -EFAULT;
        }
        //kfree(buf);
    }else{
        return -EFAULT;
    }
    
    return SUCCESS;
}
static ssize_t my_write(struct file *file, const char __user *buf,
                            size_t len, loff_t *ppos)
{
    printk(KERN_ALERT "Writing %ld bytes\n", len);
    return len;
}

static int my_close(struct inode *inodep, struct file *file)
{
    printk(KERN_ALERT "Close! \n");
    return SUCCESS;
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
int __init mytime_init(void)
{
    
    int error = misc_register(&my_misc_device);
    printk(KERN_ALERT "mytime init!\n");
    
    if (error) {
        printk(KERN_ALERT "misc_register error: %d!\n",error);
        return error;
    }
    return SUCCESS;
}



// called when module is removed
void __exit mytime_exit(void)
{
    misc_deregister(&my_misc_device);
    printk(KERN_ALERT "mytime exit!!\n");
}

module_init(mytime_init);
module_exit(mytime_exit);
