#include <linux/module.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <asm/atomic.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/interrupt.h>

static struct task_struct *kthread;
static DECLARE_WAIT_QUEUE_HEAD(wq);

static int irq = 17;	// ath9k

static atomic_t N_th = ATOMIC_INIT(0);
static atomic_t todo = ATOMIC_INIT(0);	/* key to this implementation */
static atomic_t done = ATOMIC_INIT(0);

static int devid;

/* top half */
irqreturn_t isr_top(int irq, void *dev_id)
{
	atomic_inc(&N_th);

	atomic_inc(&todo);

	pr_info("Top => jiffies: %lu, top_half: %d\n", jiffies, atomic_read(&N_th));

	wake_up_interruptible(&wq);
	
	return IRQ_NONE;
}

/* bottom half */
int isr_bottom(void *data)
{
	do {
		pr_info("INTO wait_event_interruptible...\n");
		wait_event_interruptible(wq, atomic_read(&todo) || kthread_should_stop());	/* Or perhaps waken up by signal */
		pr_info("OUT OF wait_event_interruptible...\n");
		if (kthread_should_stop())
			break;

		/*
		if (signal_pending(current))
			continue;
		*/

		/* due to signal */
		if (atomic_read(&todo) <= 0)
			continue;

		/* used to deal with more than top-half calling */
		for(;;) {
			atomic_inc(&done);
			pr_info("Loop => jiffies: %lu, done: %d\n", jiffies, atomic_read(&done));
			if (atomic_dec_and_test(&todo))
				break;
		}
	} while(!kthread_should_stop());

	return 0;
}

static int __init Nirq_init(void)
{
	int rval;

	kthread = kthread_run(isr_bottom, NULL, "kthread");

	if (!kthread) {
		pr_info("failed to create a kernel thread\n");
		return -1;
	}

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
	kthread_stop(kthread);
	synchronize_irq(irq);
	free_irq(irq, &devid);
}

module_init(Nirq_init);
module_exit(Nirq_exit);
MODULE_LICENSE("GPL");