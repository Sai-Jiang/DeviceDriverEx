
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kprobes.h>

static struct jprobe jprobe;
static unsigned long counter = 0;

static void mod_timer_inst(struct timer_list *timer, unsigned long expires)
{
	counter++;

	if (counter % 10 == 0) {
		pr_info("Counter: %lu\n", counter);
	}

	jprobe_return();
}

static int __init jprobes_init(void)
{
	int rval;

	jprobe.entry = (kprobe_opcode_t *)mod_timer_inst;
	jprobe.kp.addr = (kprobe_opcode_t *)mod_timer;
	rval = register_jprobe(&jprobe);
	if (rval) {
		pr_info("failed to register jprobe\n");
		return rval;
	}

	return 0;
}

static void __exit jprobes_exit(void)
{
	unregister_jprobe(&jprobe);
}

module_init(jprobes_init);
module_exit(jprobes_exit);
MODULE_LICENSE("GPL");