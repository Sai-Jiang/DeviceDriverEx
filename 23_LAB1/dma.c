#include <linux/module.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/slab.h>

static struct device dev;
static void *dma_buf;
static dma_addr_t dma_addr;
struct dma_pool *dmapool;



static int __init mydma_init()
{
	device_register(&dev);

	pr_info("Loading DMA allocation test module\n");

	pr_info("\nTesting dma_alloc_coherent()..........\n\n");
	dma_buf = dma_alloc_coherent(&dev,size, &dma_addr, GFP_KERNEL);

	dma_free_coherent(&dev, size, dma_buf, dma_addr);


	pr_info("\nTesting dma_map_single()................\n\n");
	dma_buf = kmalloc(size, GFP_KERNEL);
	dma_addr = dma_map_single(&dev, dma_buf, size, dir);

	dma_unmap_single(&dev, dma_addr, size, dir);
	kfree(dma_buf);

	pr_info("\nTesting dma_pool_alloc()..........\n\n");
	dmapool = dma_pool_create("mydmapool", &dev, size, align, );
	dma_buf = dma_pool_alloc(dmapool, GFP_KERNEL, &dma_addr);

	dma_pool_free(dmapool, dma_buf, dma_addr);
	dma_pool_destroy(dmapool);

	device_unregister(&dev);

	return 0;
}

static void __exit mydma_exit()
{
	pr_info("Module Unloading\n");
}

module_init(mydma_init);
module_exit(mydma_exit);
MODULE_AUTHOR("Sai.Jiang");
MODULE_LICENSE("GPL");