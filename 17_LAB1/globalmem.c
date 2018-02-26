#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/param.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>

#define GLOBALMEM_MAGIC 250
#define MEM_CLEAR _IO(GLOBALMEM_MAGIC, 0)

static int N_GLOBALMEM = 1;
module_param(N_GLOBALMEM, int , S_IRUGO | S_IWUSR);

static int GLOBALMEM_SIZE = 1024;
module_param(GLOBALMEM_SIZE, int , S_IRUGO | S_IWUSR);

static int globalmem_major = 0;
module_param(globalmem_major, int , S_IRUGO | S_IWUSR);

static struct kmem_cache *kcache = NULL;

struct globalmem_dev {
    struct cdev cdev;
    void *ramdisk;
};

struct globalmem_dev *devp;

static int globalmem_open(struct inode *inode, struct file *filp)
{
    struct globalmem_dev *devp;

    devp = container_of(inode->i_cdev, struct globalmem_dev, cdev);
    devp->ramdisk = kmem_cache_alloc(kcache, GFP_KERNEL);
    if (!devp->ramdisk) {
        pr_info("failed to allocate a cache\n");
        return -ENOMEM;
    }
    
    filp->private_data = devp;

    return 0;
}

static int globalmem_release(struct inode *inode, struct file *filp)
{
    struct globalmem_dev *devp;

    devp = filp->private_data;
    kmem_cache_free(kcache, devp->ramdisk);

    return 0;
}

static ssize_t globalmem_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    int rval;
    struct globalmem_dev *dev = filp->private_data;
    char *src = dev->ramdisk;
    unsigned long pos = *ppos;

    if (count > GLOBALMEM_SIZE)
        count = GLOBALMEM_SIZE;

    if (pos + count > GLOBALMEM_SIZE)
        count = GLOBALMEM_SIZE - pos;

    rval = count - copy_to_user(buf, src + pos, count);

    *ppos += rval;

    return rval;
}

static ssize_t globalmem_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    int rval;
    struct globalmem_dev *dev = filp->private_data;
    char *dest = dev->ramdisk;
    unsigned long pos = *ppos;

    if (count > GLOBALMEM_SIZE)
        count = GLOBALMEM_SIZE;

    if (pos + count > GLOBALMEM_SIZE)
        count = GLOBALMEM_SIZE - pos;

    rval = count - copy_from_user(dest + pos, buf, count);

    *ppos += rval;

    return rval;
}

static long globalmem_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct globalmem_dev *dev = filp->private_data;

    switch(cmd) {
    case MEM_CLEAR:
        memset(dev->ramdisk, 0, GLOBALMEM_SIZE);
        break;
    default:
        return -EINVAL;
    }

    return 0;
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = globalmem_open,
    .release = globalmem_release,
    .read = globalmem_read,
    .write = globalmem_write,
    .unlocked_ioctl = globalmem_ioctl,
};

static void globalmem_setup_cdev(void)
{
    int i, rval;
    dev_t devno;

    for (i = 0; i < N_GLOBALMEM; i++) {
        devno = MKDEV(globalmem_major, i);
        cdev_init(&devp[i].cdev, &fops);
        rval = cdev_add(&devp[i].cdev, devno, 1);
    }
}

static int __init globalmem_init(void)
{
    int rval;
    dev_t devno;

    kcache = kmem_cache_create("ramdisk", GLOBALMEM_SIZE, 0, SLAB_HWCACHE_ALIGN, NULL);
    if (!kcache) {
        pr_info("failed to create a slab cache\n");
        return -ENOMEM;
    }

    if (globalmem_major) {
        devno = MKDEV(globalmem_major, 0);
        rval = register_chrdev_region(devno, N_GLOBALMEM, "globalmem");
    }
    else {
        rval = alloc_chrdev_region(&devno, 0, N_GLOBALMEM, "globalmem");
        globalmem_major = MAJOR(devno);
    }

    if (rval < 0)
        return rval;

    devp = kzalloc(N_GLOBALMEM * sizeof(struct globalmem_dev), GFP_KERNEL);
    if (!devp) {
        unregister_chrdev_region(devno, N_GLOBALMEM);
        return -ENOMEM;
    }

    globalmem_setup_cdev();

    return 0;
}

static void __exit globalmem_exit(void)
{
    int i;
    dev_t devno;

    for (i = 0; i < N_GLOBALMEM; i++) {
        devno = MKDEV(globalmem_major, i);
        cdev_del(&devp[i].cdev);
    }

    kmem_cache_destroy(kcache);

    kfree(devp);

    devno = MKDEV(globalmem_major, 0);
    unregister_chrdev_region(devno, N_GLOBALMEM);
}


module_init(globalmem_init);
module_exit(globalmem_exit);
MODULE_LICENSE("GPL");