#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <asm/errno.h>
#include <linux/semaphore.h>

#define SUCCESS 0

MODULE_LICENSE("DUAL BSD/GPL");

#define N 10
static struct semaphore _mutex ;//lock
static struct semaphore _empty ;
static struct semaphore _full ;


static int _isOpen = 0;
static int _buffer[N];
static int _index = 0;


static int my_open(struct inode *inode, struct file *file);
static int my_close(struct inode *inodep, struct file *file);

static ssize_t my_read(struct file *file, int __user *out, size_t len, loff_t *ppos);
static ssize_t my_write(struct file *file, int __user *buf,
                        size_t len, loff_t *ppos);


static int my_open(struct inode *inode, struct file *file)
{
    
    printk(KERN_ALERT "Open!\n");
    
    if (_isOpen) return -EBUSY;
    try_module_get(THIS_MODULE);//if it fails, then the module is being removed and you should act as if it wasn't there.
    
    _isOpen = 1;
    sema_init(&_mutex, 1);
    sema_init(&_full, 0);
    sema_init(&_empty, N);

    return SUCCESS;
}
static int my_close(struct inode *inodep, struct file *file)
{
    _isOpen = 0;
    module_put(THIS_MODULE); // Decrement the usage count
    printk(KERN_ALERT "Close! \n");
    
    return SUCCESS;
}


static ssize_t my_read(struct file *file, int __user *out, size_t len, loff_t *ppos)
{
    int err = 0;
    if (access_ok(VERIFY_WRITE, out, len)) {
        down_interruptible(&_full);
        down_interruptible(&_mutex);
        int process = _buffer[_index--];
        err = copy_to_user(out,process,len);
        if (err == SUCCESS) {
            printk(KERN_ALERT "read %d, length: %d\n", process,_index);
        }else{
            printk(KERN_ALERT "Copy Error:%d\n",err);
        }
        up(&_mutex);
        up(&_empty);

    }else{
        return -EFAULT;
    }
    
    return SUCCESS;
}
static ssize_t my_write(struct file *file, int __user *buf,
                            size_t len, loff_t *ppos)
{
    
    down_interruptible(&_empty);
    down_interruptible(&_mutex);
    
    int err = copy_from_user(_buffer[_index++],buf,len);
    if (err == SUCCESS) {
        printk(KERN_ALERT "write %d, length: %d\n", buf,_index);
    }else{
        printk(KERN_ALERT "Copy Error:%d\n",err);
    }

    up(&_mutex);
    up(&_full);
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
    .name = "numpipe",
    .fops = &my_fops
};



// called when module is installed
int __init my_init(void)
{
    
    int error = misc_register(&my_misc_device);
    printk(KERN_ALERT "my init:");
    
    if (error) {
        printk(KERN_ALERT "misc_register error: %d!\n",error);
        return error;
    }
    printk(KERN_ALERT "SUCCESS\n");

    return SUCCESS;
}



// called when module is removed
void __exit my_exit(void)
{
    misc_deregister(&my_misc_device);
    printk(KERN_ALERT "Exit!!\n");
}

module_init(my_init);
module_exit(my_exit);
