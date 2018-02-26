#include <linux/init.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

static struct net_device *ndev;

static int ndev_open(struct net_device *dev)
{
	/* request irq, memory, 
	and other resources here */

	pr_info("ndev_open: %s\n", dev->name);

	netif_start_queue(dev);

	return 0;
}

static int ndev_stop(struct net_device *dev)
{
	pr_info("ndev_close: %s\n", dev->name);

	netif_stop_queue(dev);

	return 0;
}

static int
ndev_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	printk("ndev_start_xmit called\n");
	dev_kfree_skb(skb);
	return 0;
}

static struct net_device_ops ndops= {
	.ndo_open = ndev_open,
	.ndo_stop = ndev_stop,
	.ndo_start_xmit = ndev_start_xmit,
};

static int __init ndrv_init(void)
{
	int i;

	pr_info("ndrv module loaded\n");

	ndev = alloc_etherdev(0);
	ndev->netdev_ops = &ndops;

	for (i = 0; i < ETH_ALEN; i++)
		ndev->dev_addr[i] = (char)i;

	return register_netdev(ndev);
}

static void __exit ndrv_exit(void)
{
	pr_info("ndrv module unloaded\n");
	unregister_netdev(ndev);
	free_netdev(ndev);
}

module_init(ndrv_init);
module_exit(ndrv_exit);
MODULE_AUTHOR("sai_j");
MODULE_LICENSE("GPL");
