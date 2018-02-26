
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kprobes.h>

struct kprobe kprobe;

static int pre_handler(struct kprobe *p, struct pt_regs *regs)
{
	pr_info("Pre => p->addr=0x%p\n", p->addr);
	pr_info("current->comm=%s, current->pid=%d\n", current->comm, current->pid);

	return 0;
}

static void post_handler(struct kprobe *p, struct pt_regs *regs,
													unsigned long flags)
{
	pr_info("Post => p->addr=0x%p\n", p->addr);
	pr_info("current->comm=%s, current->pid=%d\n", current->comm, current->pid);
}

static int fault_handler(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
	pr_info("Fault => p->addr=0x%p\n", p->addr);
	pr_info("current->comm=%s, current->pid=%d\n", current->comm, current->pid);

	return 0;
}

static int break_handler(struct kprobe *p, struct pt_regs *regs)
{
	pr_info("Break => p->addr=0x%p\n", p->addr);
	pr_info("current->comm=%s, current->pid=%d\n", current->comm, current->pid);

	return 0;
}

static int __init kprobes_init(void)
{
	int rval;

	kprobe.symbol_name = "do_fork";
	kprobe.offset = 0;
	kprobe.pre_handler = pre_handler;
	kprobe.post_handler = post_handler;
	kprobe.fault_handler = fault_handler;
	kprobe.break_handler = break_handler;

	rval = register_kprobe(&kprobe);
	if (rval) {
		pr_info("failed to register kprobe\n");
		return rval;
	}

	return 0;
}

static void __exit kprobes_exit(void)
{
	unregister_kprobe(&kprobe);
}

module_init(kprobes_init);
module_exit(kprobes_exit);
MODULE_LICENSE("GPL");