#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/time.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <asm/errno.h>

#define SUCCESS 0

MODULE_LICENSE("DUAL BSD/GPL");


static int _isOpen = 0;



static int my_open(struct inode *inode, struct file *file);
static int my_close(struct inode *inodep, struct file *file);

static ssize_t my_read(struct file *file, char __user *out, size_t len, loff_t *ppos);
static ssize_t my_write(struct file *file, const char __user *buf,
                        size_t len, loff_t *ppos);


static int my_open(struct inode *inode, struct file *file)
{
    
    printk(KERN_ALERT "Open !\n");
    
    if (_isOpen) return -EBUSY;
    try_module_get(THIS_MODULE);//if it fails, then the module is being removed and you should act as if it wasn't there.
    
    _isOpen = 1;
    
    return SUCCESS;
}
static int my_close(struct inode *inodep, struct file *file)
{
    _isOpen = 0;
    module_put(THIS_MODULE); // Decrement the usage count
    printk(KERN_ALERT "Close! \n");
    return SUCCESS;
}


static ssize_t my_read(struct file *file, char __user *out, size_t len, loff_t *ppos)
{
    int err = 0;
    if (access_ok(VERIFY_WRITE, out, len)) {
        struct timespec time = current_kernel_time();//返回内核最后一次更新的xtime时间
        struct timespec time_day;
        getnstimeofday(&time_day);
        
    
        char buf[200];
        sprintf(buf, "current_kernel_time: %9ld %6ld\ngetnstimeofday: %9ld %6ld\n",time.tv_sec,time.tv_nsec,time_day.tv_sec,time_day.tv_nsec);
        printk(KERN_ALERT "%s\n",buf);
        err =  copy_to_user(out, buf, len);
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
    printk(KERN_ALERT "mytime init:");
    
    if (error) {
        printk(KERN_ALERT "misc_register error: %d!\n",error);
        return error;
    }
    printk(KERN_ALERT "SUCCESS\n");

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
