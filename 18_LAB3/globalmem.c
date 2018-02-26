#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/param.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/mm.h>


/*
#define GLOBALMEM_MAGIC 250
#define MEM_CLEAR _IO(GLOBALMEM_MAGIC, 0)
#define GET_SIZE _IOR(GLOBALMEM_MAGIC, 1, int)
*/

#define MEM_CLEAR   0
#define GET_SIZE    1

static int N_GLOBALMEM = 1;
module_param(N_GLOBALMEM, int , S_IRUGO | S_IWUSR);

static int globalmem_major = 0;
module_param(globalmem_major, int , S_IRUGO | S_IWUSR);

struct globalmem_dev {
    struct cdev cdev;
    void *ramdisk;
};

struct globalmem_dev *devp;

static int globalmem_open(struct inode *inode, struct file *filp)
{
    filp->private_data = container_of(inode->i_cdev, struct globalmem_dev, cdev);

    return 0;
}


static ssize_t globalmem_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    int rval;
    struct globalmem_dev *dev = filp->private_data;
    char *src = dev->ramdisk;
    unsigned long pos = *ppos;

    if (count > PAGE_SIZE)
        count = PAGE_SIZE;

    if (pos + count > PAGE_SIZE)
        count = PAGE_SIZE - pos;

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

    if (count > PAGE_SIZE)
        count = PAGE_SIZE;

    if (pos + count > PAGE_SIZE)
        count = PAGE_SIZE - pos;

    rval = count - copy_from_user(dest + pos, buf, count);

    *ppos += rval;

    return rval;

}

static int globalmem_mmap(struct file *filp, struct vm_area_struct *vma)
{
    if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
                        vma->vm_end - vma->vm_start, vma->vm_page_prot))
        return -EAGAIN;

    return 0;
}

static long globalmem_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct globalmem_dev *dev = filp->private_data;

    switch(cmd) {
    case MEM_CLEAR:
        memset(dev->ramdisk, 0, PAGE_SIZE);
        break;
    case GET_SIZE:
        put_user(PAGE_SIZE, (int __user *)arg);
        break;
    default:
        return -EINVAL;
    }

    return 0;
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = globalmem_open,
    .read = globalmem_read,
    .write = globalmem_write,
    .mmap = globalmem_mmap,
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
    int i;
    int rval;
    dev_t devno;

    if (globalmem_major) {
        devno = MKDEV(globalmem_major, 0);
        rval = register_chrdev_region(devno, N_GLOBALMEM, "globalmem");
    }
    else {
        rval = alloc_chrdev_region(&devno, 0, N_GLOBALMEM, "globalmem");
        globalmem_major = MAJOR(devno);
        pr_info("Major: %d\n", globalmem_major);
    }

    if (rval < 0)
        return rval;

    devp = kmalloc(N_GLOBALMEM * sizeof(struct globalmem_dev), GFP_KERNEL);
    if (!devp) {
        unregister_chrdev_region(devno, N_GLOBALMEM);
        return -ENOMEM;
    }

    for (i = 0; i < N_GLOBALMEM; i++)
        devp[i].ramdisk = (void *)__get_free_page(GFP_KERNEL);

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

    devno = MKDEV(globalmem_major, 0);
    unregister_chrdev_region(devno, N_GLOBALMEM);

    for (i = 0; i < N_GLOBALMEM; i++)
        kfree(devp[i].ramdisk);

    kfree(devp);
}

module_init(globalmem_init);
module_exit(globalmem_exit);
MODULE_LICENSE("GPL");