#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/param.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/completion.h>
#include <linux/sched.h>

DECLARE_COMPLETION(comp);

#define GLOBALMEM_MAGIC 250
#define MEM_CLEAR _IO(GLOBALMEM_MAGIC, 0)

static int N_GLOBALMEM = 1;
module_param(N_GLOBALMEM, int , S_IRUGO | S_IWUSR);

#define GLOBALMEM_SIZE (1024)

static int globalmem_major = 0;
module_param(globalmem_major, int , S_IRUGO | S_IWUSR);

struct globalmem_dev {
    struct cdev cdev;
    unsigned char mem[GLOBALMEM_SIZE];
};

struct globalmem_dev *devp;

int globalmem_open(struct inode *inode, struct file *filp)
{
    filp->private_data = container_of(inode->i_cdev, struct globalmem_dev, cdev);

    return 0;
}

ssize_t globalmem_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    int rval;
    unsigned long pos = *ppos;
    struct globalmem_dev *dev = filp->private_data;

    pr_info("process %i (%s) going to sleep\n", current->pid, current->comm);
    wait_for_completion(&comp);
    pr_info("process %i (%s) awakening\n", current->pid, current->comm);

    if (pos >= GLOBALMEM_SIZE)
        return 0;

    if (count > GLOBALMEM_SIZE - pos)
        count = GLOBALMEM_SIZE - pos;

    if (copy_to_user(buf, (void *)dev->mem + pos, count))
        rval = -EFAULT;
    else {
        *ppos += count;
        rval = count;
    }

    return rval;
}

ssize_t globalmem_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    int rval;
    unsigned long pos = *ppos;
    struct globalmem_dev *dev = filp->private_data;


    if (pos >= GLOBALMEM_SIZE)
        return 0;

    if (count > GLOBALMEM_SIZE - pos)
        count = GLOBALMEM_SIZE - pos;

    if (copy_from_user((void *)dev->mem + pos, buf, count))
        rval = -EFAULT;
    else {
        *ppos += count;
        rval = count;
    }

    pr_info("process %d (%s) awakening the readers...\n", current->pid, current->comm);
    complete(&comp);

    return rval;
}

long globalmem_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct globalmem_dev *dev = filp->private_data;

    switch(cmd) {
    case MEM_CLEAR:
        memset(dev->mem, 0, GLOBALMEM_SIZE);
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

    kfree(devp);

    devno = MKDEV(globalmem_major, 0);
    unregister_chrdev_region(devno, N_GLOBALMEM);
}


module_init(globalmem_init);
module_exit(globalmem_exit);
MODULE_LICENSE("GPL");

