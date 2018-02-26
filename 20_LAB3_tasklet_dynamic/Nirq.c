#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <asm/atomic.h>
#include <linux/slab.h>
#include <linux/delay.h>

static int irq = 17;	// ath9k
static atomic_t N_th = ATOMIC_INIT(0);
static atomic_t N_bh = ATOMIC_INIT(0);
static int devid;

/* bottom half */
void isr_bottom(unsigned long arg)
{
	struct tasklet_struct *tsk = (struct tasklet_struct *)arg;

	atomic_inc(&N_bh);
	pr_info("Bottom => jiffies: %lu, counter: %d\n", jiffies, atomic_read(&N_bh));
	kfree(tsk);
}

/* top half */
irqreturn_t isr_top(int irq, void *dev_id)
{
	struct tasklet_struct *tsk = kmalloc(sizeof(struct tasklet_struct), GFP_ATOMIC);

	atomic_inc(&N_th);
	pr_info("Top => jiffies: %lu, counter: %d\n", jiffies, atomic_read(&N_th));

	tasklet_init(tsk, isr_bottom, 0);
	tasklet_schedule(tsk);

	mdelay(3);

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