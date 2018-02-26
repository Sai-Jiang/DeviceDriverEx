#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/sched.h>

#define FLAG_PID	(1)
#define FLAG_SIG	(2)

static pid_t pid = -1;
static int sigid = -1;

static struct task_struct *task = NULL;

static struct proc_dir_entry *rx_PID = NULL;
static struct proc_dir_entry *tx_sig = NULL;

static int read_entry(char *page, char **start, off_t off, 
						int count, int *eof, void *data)
{
	int rval;
	int flag = (int)data;

	switch(flag) {
		case FLAG_SIG:
			rval = sprintf(page, "tx_sig: %d\n", sigid);
			break;
		case FLAG_PID:
			rval = sprintf(page, "rx_PID: %d\n", pid);
			break;
		default:
			rval = sprintf(page, "Something wrong!\n");
			break;
	}

	return rval;
}

static int write_entry(struct file *filp, const char __user *buffer, 
						unsigned long count, void *data)
{
	char buf[8];
	int n;
	long val;
	int flag = (int)data;

	/* a little buggy */
	/* assume count is less than 8 */

	n = count - copy_from_user(buf, buffer, count);
	val = simple_strtol(buf, NULL, 0);
	pr_info("DEBUG => val: %ld", val);


	switch(flag) {

	case FLAG_SIG:
		sigid = val;
		if (task) 
			send_sig(sigid, task, 0);
		break;
	case FLAG_PID:
		pid = val;
		task = pid_task(find_get_pid(pid), PIDTYPE_PID); 
		break;
	default:
		pr_info("Something wrong!\n");
		break;
	}

	return n;
}

static int __init proc_signal_init(void)
{
	rx_PID = create_proc_entry("rx_PID", 0644, NULL);
	if (!rx_PID) {
		pr_info("failed to create rx_PID entry\n");
		return -ENOMEM;
	}
	rx_PID->read_proc = read_entry;
	rx_PID->write_proc = write_entry;
	rx_PID->data = (void *)FLAG_PID;

	tx_sig = create_proc_entry("tx_sig", 0644, NULL);
	if (!tx_sig) {
		pr_info("failed to create tx_sig entry\n");
		return -ENOMEM;
	}
	tx_sig->read_proc = read_entry;
	tx_sig->write_proc = write_entry;
	tx_sig->data = (void *)FLAG_SIG;

	return 0;
}

static void __exit proc_signal_exit(void)
{
	if (rx_PID)
		remove_proc_entry("rx_PID", NULL);
	if (tx_sig)
		remove_proc_entry("tx_sig", NULL);
}

module_init(proc_signal_init);
module_exit(proc_signal_exit);
MODULE_LICENSE("GPL");