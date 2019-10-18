#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/i2c.h>

/*
 * The probe function is called each time this driver is instantiated to manage
 * a device. It is responsible for preparing the device for use.
 */
int opt3002_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	/* dev_info is similar to printk except it's associated with a device */
	dev_info(&client->dev,
		 "In probe() for device: {name=%s, driver_data=%lu}\n",
		 id->name, id->driver_data);
	return 0;
}

/*
 * The remove function is called each time a device managed by this driver is
 * removed. It should clean up any resources allocated by the driver.
 */
int opt3002_remove(struct i2c_client *client)
{
	dev_info(&client->dev, "In remove()\n");
	return 0;
}

/*
 * The list of I2C device IDs that are supported by this driver.
 *
 * == TASK ==
 * Add a '{ "opt3002", 0 },' entry to declare that this driver supports the
 * opt3002. Look at linux/i2c.h if you want to understand the types of the
 * fields within struct i2c_device_id.
 */
static const struct i2c_device_id opt3002_id_table[] = {
	{} /* Empty entry required to terminate the list */
};
/* Advertise the device ID table from this module */
MODULE_DEVICE_TABLE(i2c, opt3002_id_table);

/*
 * The i2c_driver defines the functions and data structures that implement the
 * driver. There are type-specific structs for other types of drivers (e.g. SPI)
 */
struct i2c_driver opt3002_driver = {
	.probe = opt3002_probe,
	.remove = opt3002_remove,
	.id_table = opt3002_id_table,
	.driver = { /* NOTE: type is struct device_driver */
		.name = "opt3002",
	},
};

/*
 * == TASK ==
 * We have defined an I2C driver structure above, but we haven't
 * actually registered the driver with the kernel. The functions i2c_add_driver/i2c_del_driver can
 * be called in the init/exit functions to register/deregister the driver. Since this is such a
 * common pattern for I2C drivers, a convenience macro exists named module_i2c_driver. This macro
 * will generate appropriate init/exit function that performs driver registration. Use
 * module_i2c_driver to replace the explicit init/exit functions below.
 */

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
