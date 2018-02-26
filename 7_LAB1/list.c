#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/kernel.h>

#define N	(8)

LIST_HEAD(list);

struct mylist {
	int val;
	struct list_head list;
};

static int __init list_init(void)
{
	int i;
	struct mylist *tmp;
	struct list_head *node;

	struct list_head *start;

	pr_info("\nAdding elements into list");
	for(i = 0; i < N; i++) {
		tmp = kzalloc(sizeof(struct mylist), GFP_KERNEL);
		tmp->val = i;
		list_add_tail(&tmp->list, &list);
	}

	pr_info("Walk through the list\n\n");
	list_for_each(node, &list) {
		tmp = list_entry(node, struct mylist, list);
		pr_info("Current: %d\n", tmp->val);
		if (tmp->val == 5)
			start = node;
	}

	list_for_each(node, start) {
		tmp = list_entry(node, struct mylist, list);
		pr_info("Current: %d\n", tmp->val);
		if (tmp->val == 5)
	}

	return 0;

}

static void __exit list_exit(void)
{
	struct list_head *node;
	struct list_head *p;
	struct mylist *tmp;

	pr_info("\nBegin to del list\n");

	list_for_each_safe(node, p, &list) {
		tmp = list_entry(node, struct mylist, list);
		list_del(&tmp->list);
		kfree(tmp);
	}

	if (list_empty(&list))
		pr_info("List empty\n");
	else
		pr_info("List not empty\n");
}
module_init(list_init);
module_exit(list_exit);
MODULE_AUTHOR("sai.jiang");
MODULE_LICENSE("GPL");