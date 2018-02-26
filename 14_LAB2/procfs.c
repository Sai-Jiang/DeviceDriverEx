#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>

#define STR_lEN	(128)

static struct proc_dir_entry *entry = NULL;

static char str[STR_lEN] = "hello";
static int n = sizeof("hello");

static int read_entry(char *page, char **start, off_t off, 
						int count, int *eof, void *data)
{
	return sprintf(page, "%s\n", str);
}

static int write_entry(struct file *filp, const char __user *buffer, 
						unsigned long count, void *data)
{
	memset(str, 0, STR_lEN);
	
	if (count > STR_lEN)
		count = STR_lEN;

	n = count - copy_from_user(str, buffer, count);

	return n;
}

static int __init procfs_init(void)
{
	/* entry = create_proc_entry("myproc", 0644, NULL); */
	entry = create_proc_entry("driver/myproc", 0644, NULL);	
	if (!entry) {
		pr_info("failed to create proc entry\n");
		return -ENOMEM;
	}
	entry->read_proc = read_entry;
	entry->write_proc = write_entry;
	pr_info("Succeed to create proc entry\n");
}

static void __exit procfs_exit(void)
{
	if (entry)
		remove_proc_entry("myproc", NULL);
}

module_init(procfs_init);
module_exit(procfs_exit);
MODULE_LICENSE("GPL");