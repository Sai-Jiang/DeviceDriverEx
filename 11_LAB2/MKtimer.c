#include <linux/module.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/kernel.h>

#define SIZE (128)

struct timer {
	struct list_head l;
	struct timer_list timer;
};

struct MKtimer {
	struct miscdevice misc;
	struct list_head head;	/* used for timer linked list head */
	/* perhaps a spinlock here to protect linked list */
};

static struct MKtimer MKtimer;

static void timer_func(unsigned long data)
{
	struct timer *tmp;

	pr_info("\nIn timer function - PID: %d\n", current->pid);
	pr_info("Deleting timer and Freeing Space...\n");
	tmp = container_of(MKtimer.head.next, struct timer, l);

	/* a lock here. Softirq => spinlock */
	list_del(MKtimer.head.next);

	kfree(tmp);
}

static ssize_t
MKtimer_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
	int n;
	char rcv[SIZE];
	struct timer *t;

	if (count > SIZE)
		count = SIZE;

	n = count - copy_from_user(rcv, buf, count);
	rcv[n] = '\0';
	pr_info("\nGet Message: %s", rcv);

	pr_info("Current PID: %d", current->pid);

	pr_info("Adding a new timer\n");
	t = kzalloc(sizeof(struct timer), GFP_KERNEL);
	init_timer(&t->timer);
	t->timer.function = timer_func;
	t->timer.expires = jiffies + 10 * HZ;

	/* Better get a lock here */
	list_add_tail(&t->l, &MKtimer.head);

	add_timer(&t->timer);

	return n;
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.write = MKtimer_write,
};

static int __init MKtimer_init(void)
{
	int rval;

	MKtimer.misc.minor = MISC_DYNAMIC_MINOR;
	MKtimer.misc.name = "timers";
	MKtimer.misc.fops = &fops;
	rval = misc_register(&MKtimer.misc);
	if (rval) {
		pr_info("failed to register misc device");
		return rval;
	}
	printk("Got minor %d\n", MKtimer.misc.minor);

	INIT_LIST_HEAD(&MKtimer.head);

	return 0;
}

static void __exit MKtimer_exit(void)
{
	misc_deregister(&MKtimer.misc);
}

module_init(MKtimer_init);
module_exit(MKtimer_exit);
MODULE_LICENSE("GPL");