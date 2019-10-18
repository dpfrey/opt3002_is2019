#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/i2c.h>

int opt3002_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	dev_info(&client->dev,
		 "In probe() for device: {name=%s, driver_data=%lu}\n",
		 id->name, id->driver_data);
	return 0;
}

int opt3002_remove(struct i2c_client *client)
{
	dev_info(&client->dev, "In remove()\n");
	return 0;
}

static const struct i2c_device_id opt3002_id_table[] = {
	{ "opt3002", 0 },
	{} /* Empty entry required to terminate the list */
};
MODULE_DEVICE_TABLE(i2c, opt3002_id_table);

struct i2c_driver opt3002_driver = {
	.probe = opt3002_probe,
	.remove = opt3002_remove,
	.id_table = opt3002_id_table,
	.driver = {
		.name = "opt3002",
	},
};
module_i2c_driver(opt3002_driver);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("David Frey <dfrey@sierrawireless.com>");
MODULE_DESCRIPTION("TI opt3002 light sensor driver");
