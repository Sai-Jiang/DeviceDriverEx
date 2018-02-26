#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>

//static int irq = 17;	// ath9k
static int irq = 19;	// vm
static int top_counter = 0;
static int half_counter = 0;
static int devid;

/* bottom half */
void isr_bottom(struct work_struct *arg)
{
	bottom_counter++;
	pr_info("Bottom => jiffies: %lu, counter: %d\n", jiffies, bottom_counter);
}

static DECLARE_WORK(irq_wq, isr_bottom);

/* top half */
irqreturn_t isr_top(int irq, void *dev_id)
{
	top_counter++;
	pr_info("Top => jiffies: %lu, counter: %d\n", jiffies, top_counter);

	schedule_work(&irq_wq);

	return IRQ_NONE;
}

static int __init Nirq_init(void)
{
	int rval;

	rval = request_irq(irq, isr_top, IRQF_SHARED, "irq_counter", &devid);
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