/* **************** LDD:2.0 s_04/sample_driver.c **************** */
/*
 * The code herein is: Copyright Jerry Cooperstein, 2012
 *
 * This Copyright is retained for the purpose of protecting free
 * redistribution of source.
 *
 *     URL:    http://www.coopj.com
 *     email:  coop@coopj.com
 *
 * The primary maintainer for this code is Jerry Cooperstein
 * The CONTRIBUTORS file (distributed with this
 * file) lists those known to have contributed to the source.
 *
 * This code is distributed under Version 2 of the GNU General Public
 * License, which you should have received with the source.
 *
 */
/* 
Sample Character Driver 
@*/

#include <linux/module.h>	/* for modules */
#include <linux/fs.h>		/* file_operations */
#include <linux/uaccess.h>	/* copy_(to,from)_user */
#include <linux/init.h>		/* module_init, module_exit */
#include <linux/slab.h>		/* kmalloc */
#include <linux/cdev.h>		/* cdev utilities */
#include <linux/kernel.h>
#include <linux/moduleparam.h>


#define MYDEV_NAME "mycdrv"

static int MAJOR = 0;
module_param(MAJOR, int, S_IRUGO | S_IWUSR);

static int MINOR = 0;
module_param(MINOR, int, S_IRUGO | S_IWUSR);

static unsigned int N_DEVICE = 1;
module_param(N_DEVICE, unsigned int, S_IRUGO | S_IWUSR);

static unsigned int ramdisk_size = 16 * PAGE_SIZE;
module_param(RAMDISK_SIZE, unsigned int, S_IRUGO | S_IWUSR);

struct mycdrv {
	struct cdev *cdev;
	char *ramdisk;
};

static int mycdrv_open(struct inode *inode, struct file *file)
{
	pr_info(" OPENING device: %s:\n", MYDEV_NAME);
	pr_info(" HAS BEEN OPENED %d times\n", module_refcount(THIS_MODULE));
	pr_info(" Major ID: %d, Minor ID %d\n", my_major, my_minor);

	pr_info(" Allocating ramdisk\n\n");
	file->private_data = kzalloc(ramdisk_size, GFP_KERNEL);

	return 0;
}

static int mycdrv_release(struct inode *inode, struct file *file)
{
	pr_info(" CLOSING device: %s:\n", MYDEV_NAME);
	pr_info(" Freeing ramdisk\n\n");
	kfree(file->private_data);
	return 0;
}

static ssize_t
mycdrv_read(struct file *file, char __user * buf, size_t lbuf, loff_t * ppos)
{
	int nbytes;
	char *ramdisk = file->private_data;

	if ((lbuf + *ppos) > ramdisk_size) {
		pr_info("trying to read past end of device,"
			"aborting because this is just a stub!\n");
		return 0;
	}
	nbytes = lbuf - copy_to_user(buf, ramdisk + *ppos, lbuf);
	*ppos += nbytes;
	pr_info("\n READING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);

	return nbytes;
}

static ssize_t
mycdrv_write(struct file *file, const char __user * buf, size_t lbuf,
	     loff_t * ppos)
{
	int nbytes;
	char *ramdisk = file->private_data;

	if ((lbuf + *ppos) > ramdisk_size) {
		pr_info("trying to read past end of device,"
			"aborting because this is just a stub!\n");
		return 0;
	}
	nbytes = lbuf - copy_from_user(ramdisk + *ppos, buf, lbuf);
	*ppos += nbytes;
	pr_info("\n WRITING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);

	return nbytes;
}

static loff_t mycdrv_llseek(struct file *file, loff_t off, int whence)
{
	int rval;

	switch(whence){
		case SEEK_SET:
			file->f_pos = off;
			break;
		case SEEK_CUR:
			file->f_pos += off;
			break;
		case SEEK_END:
			file->f_pos += 
		default:
			break;
	}
}

static const struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
	.open = mycdrv_open,
	.release = mycdrv_release,
	.read = mycdrv_read,
	.write = mycdrv_write,
	.lseek = mycdrv_lseek,
};

static int __init my_init(void)
{
	my_cdev = cdev_alloc();
	cdev_init(my_cdev, &mycdrv_fops);
	cdev_add(my_cdev, first, count);
	pr_info("\nSucceeded in registering character device %s\n", MYDEV_NAME);
	return 0;

	dev_t first;

	if (MAJOR) {
		first = MKDEV(MAJOR, MINOR);
		register_chrdev_region(first, N_DEVICE, MYDEV_NAME);
	}else {
		alloc_chrdev_region(&first, MINOR, N_DEVICE, MYDEV_NAME);
	}

	
}

static void __exit my_exit(void)
{
	cdev_del(my_cdev);
	unregister_chrdev_region(first, count);
	pr_info("\ndevice unregistered\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_LICENSE("GPL v2");