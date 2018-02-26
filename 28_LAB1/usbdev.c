#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb.h>
#include <linux/kernel.h>

#define VENDOR_ID (0x17ef)
#define PRODUCT_ID (0x6039)

static struct usb_device_id usbdev_table[] = {
	{ USB_DEVICE(VENDOR_ID, PRODUCT_ID) },
	{}
};

MODULE_DEVICE_TABLE(usb, usbdev_table);

int usbdev_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(intf);

	pr_info("\nusbdev_probe\n");
	pr_info("devnum=%d, speed=%d\n", dev->devnum, (int)dev->speed);
	pr_info("idVendor=0x%hX, idProduct=0x%hX, bcdDevice=0x%hX\n",
		dev->descriptor.idVendor,
		dev->descriptor.idProduct, dev->descriptor.bcdDevice);
	pr_info("class=0x%hX, subclass=0x%hX\n",
		dev->descriptor.bDeviceClass, dev->descriptor.bDeviceSubClass);
	pr_info("protocol=0x%hX, packetsize=%hu\n",
		dev->descriptor.bDeviceProtocol,
		dev->descriptor.bMaxPacketSize0);
	pr_info("manufacturer=0x%hX, product=0x%hX, serial=%hu\n",
		dev->descriptor.iManufacturer, dev->descriptor.iProduct,
		dev->descriptor.iSerialNumber);

	return 0;
}

void usbdev_discon(struct usb_interface *intf)
{
	pr_info("usbdev => device disconnect\n");
}

static struct usb_driver usbdev_drv = {
	.name = "usbdev",
	.id_table = usbdev_table,
	.probe = usbdev_probe,
	.disconnect = usbdev_discon,
};

static int __init usbdev_init(void)
{
	int rval;

	rval = usb_register(&usbdev_drv);
	if (rval) {
		pr_info("failed to register a usb driver\n");
		return -1;
	}

	pr_info("usbdev module loaded\n");
	return 0;
}

static void __exit usbdev_exit(void)
{
	usb_deregister(&usbdev_drv);
	pr_info("usbdev unloaded\n");
}

module_init(usbdev_init);
module_exit(usbdev_exit);
MODULE_LICENSE("GPL");
