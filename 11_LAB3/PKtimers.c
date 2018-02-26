#include <linux/module.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/fs.h>

#define SIZE (128)

struct timers {
	struct miscdevice misc;
	struct timer_list timer;
};

struct timers timers;

void timer_func(unsigned long data)
{
	pr_info("\nIn timer function - PID: %d\n", current->pid);
	mod_timer(&timers.timer, jiffies + HZ);
}

static ssize_t
timers_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
	int n;
	char rcv[SIZE];

	if (count > SIZE)
		count = SIZE;

	n = count - copy_from_user(rcv, buf, count);
	pr_info("\nGet Message: %s", rcv);

	pr_info("Current PID: %d", current->pid);
	pr_info("Launch a timer\n");
	init_timer(&timers.timer);
	timers.timer.function = timer_func;
	timers.timer.expires = jiffies + HZ;
	add_timer(&timers.timer);

	return n;
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.write = timers_write,
};

static int __init timers_init(void)
{
	int rval;

	timers.misc.minor = MISC_DYNAMIC_MINOR;
	timers.misc.name = "timers";
	timers.misc.fops = &fops;
	rval = misc_register(&timers.misc);
	if (rval) {
		pr_info("failed to register misc device");
		return rval;
	}
	printk("Got minor %d\n", timers.misc.minor);

	return 0;
}

static void __exit timers_exit(void)
{
	misc_deregister(&timers.misc);
}

module_init(timers_init);
module_exit(timers_exit);
MODULE_LICENSE("GPL");