#include <linux/module.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>

#define SIZE (128)

struct dev {
	struct miscdevice misc;
	struct work_struct work;
	char buf[SIZE];
};

static struct dev mydev;

/* race condition !!! */
static void wq_func(struct work_struct *arg)
{
	struct dev *mydevp = container_of(arg, struct dev, work);

	pr_info("wq_func - PID: %d\n", current->pid);
	pr_info("Current jiffies: %lu\n", jiffies);
	pr_info("what i get from write: %s\n", mydevp->buf);
}

static ssize_t mydev_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
	int n;

	if (count > SIZE)
		count = SIZE;

	n = count - copy_from_user(mydev.buf, buf, count);

	pr_info("Current PID: %d\n", current->pid);
	pr_info("Current jiffies: %lu\n", jiffies);
	schedule_work(&mydev.work);
	pr_info("Current jiffies: %lu\n", jiffies);
	return n;
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.write = mydev_write,
};

static int __init mydev_init(void)
{
	int rval;

	mydev.misc.minor = MISC_DYNAMIC_MINOR;
	mydev.misc.name = "workqueue";
	mydev.misc.fops = &fops;
	rval = misc_register(&mydev.misc);
	if (rval) {
		pr_info("failed to register misc device");
		return rval;
	}
	printk("Got minor %d\n", mydev.misc.minor);

	INIT_WORK(&mydev.work, wq_func);

	return 0;
}

static void __exit mydev_exit(void)
{
	misc_deregister(&mydev.misc);
}

module_init(mydev_init);
module_exit(mydev_exit);
MODULE_LICENSE("GPL");