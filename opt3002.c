#include <linux/module.h>
#include <linux/kernel.h>

/*
 * This "init" function is run when the module is loaded into the kernel.
 * Currently, this function only returns the value 0. A function that returns an
 * int in the kernel often means that it returns either 0 (or perhaps some
 * positive count or length) on success. If a negative value is returned, then
 * it typically represents a negative errno. The standard errno values are
 * defined in include/uapi/asm-generic/errno.h. __init is a macro which places
 * the function in a specific linker section. This can be omitted, but it's good
 * practice to include it.
 *
 * == TASK ==
 * The kernel doesn't have access to the standard C library. Instead of printf,
 * the kernel provides printk for logging messages. Make this module print a
 * message when it's loaded and unloaded.
 */
int __init opt3002_init(void)
{
	return 0;
}
/* Tells the kernel to call this function when the module is loaded */
module_init(opt3002_init);

/*
 * This exit function is run when the module is unloaded. Unlike the init
 * function, the exit function cannot fail and thus the return type is void.
 */
void __exit opt3002_exit(void)
{
}
/* Tells the kernel to call this function when the module is unloaded */
module_exit(opt3002_exit);
