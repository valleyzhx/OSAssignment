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

//#define N 10
static int N = 10;
module_param(N, int, S_IRUGO);



static struct semaphore _mutex;//lock
static struct semaphore _empty;
static struct semaphore _full;

//static struct semaphore _index_lock;

static int _open_count = 0;
static int *_buffer;
static int _count = 0;
//static int *_temp;



static int my_open(struct inode *inode, struct file *file);
static int my_close(struct inode *inodep, struct file *file);

static ssize_t my_read(struct file *file, int __user *out, size_t len, loff_t *ppos);
static ssize_t my_write(struct file *file, int __user *buf,
                        size_t len, loff_t *ppos);


static int my_open(struct inode *inode, struct file *file)
{
    
    printk(KERN_ALERT "Open!\n");
    
    try_module_get(THIS_MODULE);//if it fails, then the module is being removed and you should act as if it wasn't there.
    
    _open_count ++;

    return SUCCESS;
}
static int my_close(struct inode *inodep, struct file *file)
{
    _open_count--;
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
        
        int process = _buffer[0];
        err = copy_to_user(out,&process,len);
        if (err == SUCCESS) {
            //down_interruptible(&_index_lock);
            int i=0;
            for (i=0; i<_count-1; i++) {
                _buffer[i] = _buffer[i+1];
            }
            _count--;
            //up(&_index_lock);

            printk(KERN_ALERT "read from %d, now items number: %d\n", process,_count);
        }else{
            printk(KERN_ALERT "Copy Error:%d\n",err);
        }
        up(&_mutex);
        up(&_empty);

    }else{
        return -EFAULT;
    }
    
    return len;
}
static ssize_t my_write(struct file *file, int __user *buf,
                            size_t len, loff_t *ppos)
{
    
    down_interruptible(&_empty);
    down_interruptible(&_mutex);
    int process;
    int err = copy_from_user(&process,buf,len);
    if (err == SUCCESS) {
        //down_interruptible(&_index_lock);
        _buffer[_count] = process;
        _count++;
        //up(&_index_lock);

        printk(KERN_ALERT "write process %d, now items number: %d\n", process,_count);
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
    sema_init(&_mutex, 1);
    sema_init(&_full, 0);
    sema_init(&_empty, N);
    //sema_init(&_index_lock, 1);

    _buffer = (int*)kmalloc(N*sizeof(int),GFP_KERNEL);
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
    kfree(_buffer);
    printk(KERN_ALERT "Exit!!\n");
}

module_init(my_init);
module_exit(my_exit);
