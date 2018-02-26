#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>

#define STR_lEN	(128)

static struct proc_dir_entry *dir = NULL;
static struct proc_dir_entry *entry0 = NULL;

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
	dir = proc_mkdir("myprocdir", NULL);
	dir->read_proc = read_entry;
	dir->write_proc = write_entry;

	entry0 = create_proc_entry("myprocdir/myproc0", 0644, NULL);
	entry0->read_proc = read_entry;
	entry0->write_proc = write_entry;
}

static void __exit procfs_exit(void)
{
	if (dir)
		remove_proc_entry("myprocdir", NULL);
	if (entry0)
		remove_proc_entry("myprocdir/myproc0", NULL);
}

module_init(procfs_init);
module_exit(procfs_exit);
MODULE_LICENSE("GPL");