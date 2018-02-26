#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

static int irq = 17;
static int irq_counter = 0;
static int devid;

irqreturn_t irq_handler(int irq, void *dev_id)
{
	irq_counter++;
	pr_info("In the ISR: counter = %d\n", irq_counter);
	return IRQ_NONE;
}

static int __init Nirq_init(void)
{
	int rval;

	rval = request_irq(irq, irq_handler, IRQF_SHARED, "irq_counter", &devid);
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
	pr_info("Total: %d\n", irq_counter);
}

module_init(Nirq_init);
module_exit(Nirq_exit);
MODULE_LICENSE("GPL");