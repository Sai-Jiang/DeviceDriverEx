#include <linux/module.h>
#include <linux/init.h>
#include <linux/>

static int maxirq = 31;
static int irq_counter = 0;
static int devid;

const static struct file_operations {
	.owner = THIS_MODULE;
	.read = Nirq_read;
}

static ssize_t Nirq_read(struct file *filp, char __user * buf, size_t count, loff_t * ppos)
{

}

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