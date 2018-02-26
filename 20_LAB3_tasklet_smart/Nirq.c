#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <asm/atomic.h>
#include <linux/delay.h>

static int irq = 17;	// ath9k

static atomic_t N_th = ATOMIC_INIT(0);
static atomic_t todo = ATOMIC_INIT(0);	/* key to this implementation */
static atomic_t done = ATOMIC_INIT(0);

static int devid;

/* bottom half */
void isr_bottom(unsigned long arg)
{
	if (atomic_read(&todo) <= 0)
		return;

	for( ; ; ) {
		atomic_inc(&done);

		pr_info("Bottom => jiffies: %lu, counter: %d\n", jiffies, atomic_read(&done));

		if (atomic_dec_and_test(&todo))
			break;
	}
}

static DECLARE_TASKLET(tsk, isr_bottom, 0);

/* top half */
irqreturn_t isr_top(int irq, void *dev_id)
{
	atomic_inc(&todo);
	atomic_inc(&N_th);

	pr_info("Top => jiffies: %lu, top_half: %d\n", jiffies, atomic_read(&N_th));

	mdelay(3);

	tasklet_schedule(&tsk);

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