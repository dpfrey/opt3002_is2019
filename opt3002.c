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

/*
 * The header file linux/module.h provides MODULE_LICENSE, MODULE_AUTHOR and
 * MODULE_DESCRIPTION macros to allow the author to specify metadata about the
 * module. Each of these macros expects a single string literal argument.
 *
 * == TASK ==
 * Define metadata for describing this module using each of the above macros.
 *
 * == TASK ==
 * On your mangOH, run "modinfo /legato/systems/current/modules/opt3002.ko" and
 * observe that the metadata is displayed.
 */
