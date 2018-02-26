#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/jiffies.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linxu/proc_fs.h>

static int items = 3;
module_param(items, int, 0644);

static int delay = 1;
module_param(delay, int , 0644);

static struct proc_dir_entry *x_busy;

/* return a pointer to the first item */
void *xbusy_start(struct seq_file *m, loff_t *pos)
{
	void *ptr;
	unsigned long later;

	if (*pos < items) {
		later = jiffies + delay * HZ;
		while(time_before(jiffies, later))
			;
		ptr = (void *)&jiffies;
	}else
		ptr = NULL;

	return ptr;
}

/* place data into seq_file data structure */
int xbusy_show(struct seq_file *m, void *v)
{
	volatile unsigned long * const j = (volatile unsigned long *)v;
	
	return seq_printf(m, "jiffies: %lu\n", *j);
}


/* return a pointer to next item. also (*pos)++ */
void *xbusy_next(struct seq_file *m, void *v, loff_t *pos)
{
	void *ptr;
	unsigned long later;

	(*pos)++;

	if (*pos < items) {
		later = jiffies + delay * HZ;
		while(time_before(jiffies, later))
			;
		ptr = (void *)&jiffies;
	}else
		ptr = NULL;

	return ptr;
}

void xbusy_stop(struct seq_file *m, void *v)
{
	/* deos nothing for us */
}


static const struct seq_operations seq_ops = {
	.start = xbusy_start,
	.show = xbusy_show,
	.next = xbusy_next,
	.stop = xbusy_stop,
};

static int xbusy_open(struct inode *inode, struct file *filp)
{
	return seq_open(filp, &seq_ops);
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = xbusy_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

static int __init xbusy_init(void)
{
	x_busy = create_proc_entry("x_busy", 0, NULL);
	x_busy->proc_fops = &fops;

	return 0;
}

static void __exit xbusy_exit(void)
{
	if (x_busy)
		remove_proc_entry("x_busy", NULL);
}

module_init(xbusy_init);
module_exit(xbusy_exit);
MODULE_LICENSE("GPL v2");