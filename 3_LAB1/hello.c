#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>

static int N = 1;
module_param(N, int, S_IRUGO | S_IWUSR);

static int __init hello_init(void)
{
	int i;

	for(i = 0; i < N; i++)
		printk(KERN_DEBUG"Hello World!");

	return 0;
}

static void __exit hello_exit(void)
{
	int i;
	
	for(i = 0; i < N; i++)
		printk(KERN_DEBUG"Say Goodbye!\n");
}

MODULE_AUTHOR("sai.jiang");
MODULE_LICENSE("GPL");

module_init(hello_init);
module_exit(hello_exit);