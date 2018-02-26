#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>

static int irq = 17;	// ath9k
static int top_counter = 0;
static int bottom_counter = 0;
static int devid;

/* bottom half */
irqreturn_t isr_bottom(int irq,void *dev_id)
{
	bottom_counter++;
	pr_info("Bottom => jiffies: %lu, counter: %d\n", jiffies, bottom_counter);

	return 0;
}

/* top half */
irqreturn_t isr_top(int irq, void *dev_id)
{
	top_counter++;
	pr_info("Top => jiffies: %lu, counter: %d\n", jiffies, top_counter);

	return IRQ_WAKE_THREAD;
}

static int __init Nirq_init(void)
{
	int rval;

	rval = request_threaded_irq(irq, isr_top, isr_bottom, IRQF_SHARED, "irq_counter", &devid);
	if (rval) {
		pr_info("failed to request irq\n");
		return -1;
	}
	pr_info("Successful to Loading module\n");
	return 0;
}

static void __exit Nirq_exit(void)
{
	synchronize_irq(irq);
	free_irq(irq, &devid);
}

module_init(Nirq_init);
module_exit(Nirq_exit);
MODULE_LICENSE("GPL");