#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/param.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/rwsem.h>
#include <linux/pagemap.h>

#define GLOBALMEM_MAGIC (250)
#define MEM_CLEAR _IO(GLOBALMEM_MAGIC, 0)

#define order 0

static int GLOBALMEM_SIZE = (PAGE_SIZE << order);
module_param(GLOBALMEM_SIZE, int , S_IRUGO | S_IWUSR);

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

static ssize_t globalmem_rw(struct file *filp, unsigned long buf, size_t count, loff_t *ppos, int rw)
{
    void *ubuf, *kbuf;
    int i, rc, nbytes, npages;
    struct page **pages;
    struct globalmem_dev *dev;

    dev = filp->private_data;
    kbuf = dev->ramdisk;

    npages = (count - 1) / PAGE_SIZE + 1;
    pages = kmalloc(npages * sizeof(struct page *), GFP_KERNEL);

    down_read(&current->mm->mmap_sem);
    rc = get_user_pages(current, current->mm, buf, npages, 1, 0, pages, NULL);
    up_read(&current->mm->mmap_sem);


    for(i = 0, nbytes = PAGE_SIZE; i < rc; i++, kbuf += PAGE_SIZE) {
        ubuf = kmap(pages[i]);

        if (i == rc - 1)
            nbytes = (count - 1) % PAGE_SIZE + 1;

        switch(rw) {
        case 0:
            memcpy(ubuf, kbuf, nbytes);
            break;
        case 1:
            memcpy(kbuf , ubuf, nbytes);
            break;
        default:
            pr_info("Something Wrong!\n");
            break;
        }

        lock_page(pages[i]);
        set_page_dirty(pages[i]);
        unlock_page(pages[i]);
        page_cache_release(pages[i]);

        kunmap(pages[i]);
    }

    kfree(pages);

    return count;
}

static ssize_t globalmem_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    return globalmem_rw(filp, (unsigned long)buf, count, ppos, 0);
}

static ssize_t globalmem_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    return globalmem_rw(filp, (unsigned long)buf, count, ppos, 1);
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = globalmem_open,
    .read = globalmem_read,
    .write = globalmem_write,
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
    }

    if (rval < 0)
        return rval;

    devp = kzalloc(N_GLOBALMEM * sizeof(struct globalmem_dev), GFP_KERNEL);
    if (!devp) {
        unregister_chrdev_region(devno, N_GLOBALMEM);
        return -ENOMEM;
    }

    for (i = 0; i < N_GLOBALMEM; i++)
        devp[i].ramdisk = (void *)__get_free_pages(GFP_KERNEL, order);

    globalmem_setup_cdev();

    return 0;
}

static void __exit globalmem_exit(void)
{
    int i;
    dev_t devno;

    for (i = 0; i < N_GLOBALMEM; i++)
        cdev_del(&devp[i].cdev);

    devno = MKDEV(globalmem_major, 0);
    unregister_chrdev_region(devno, N_GLOBALMEM);

    for (i = 0; i < N_GLOBALMEM; i++)
        free_pages((unsigned long)devp[i].ramdisk, order);

    kfree(devp);
}

module_init(globalmem_init);
module_exit(globalmem_exit);
MODULE_LICENSE("GPL");