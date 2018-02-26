/**
 *	struct module {
 *		...
 *		struct list_head modules;
 *		...
 *		char name[MODULE_NAME_LEN];
 *		...
 *		unsigned int taints;	
 *		...
 *	}	
 *
 *
 **/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/kernel.h>

static int __init walkmodule_init(void)
{
	struct module *pmod;

	list_for_each_entry(pmod, THIS_MODULE->list.prev, list) {
		pr_info("module name: %s, taints: %u\n", pmod->name, pmod->taints);
	}
	
	return 0;
}

static void __exit walkmodule_exit(void)
{

}

module_init(walkmodule_init);
module_exit(walkmodule_exit);
MODULE_AUTHOR("sai.jiang");
MODULE_LICENSE("GPL");
