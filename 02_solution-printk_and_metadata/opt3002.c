#include <linux/module.h>
#include <linux/kernel.h>

int __init opt3002_init(void)
{
	printk("Hello opt3002!\n");
	return 0;
}
module_init(opt3002_init);

void __exit opt3002_exit(void)
{
	printk("Goodbye opt3002!\n");
}
module_exit(opt3002_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("David Frey <dfrey@sierrawireless.com>");
MODULE_DESCRIPTION("TI opt3002 light sensor driver");
