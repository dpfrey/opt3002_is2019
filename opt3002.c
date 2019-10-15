#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/i2c.h>

/*
 * Texas Instruments OPT3002 Ambient Light Sensor driver
 * Datasheet: http://www.ti.com/lit/ds/symlink/opt3002.pdf
 */
#define OPT3002_REG_RESULT		0x00
#define OPT3002_REG_CONFIGURATION	0x01
#define OPT3002_REG_LOW_LIMIT		0x02
#define OPT3002_REG_HIGH_LIMIT		0x03
#define OPT3002_REG_MANUFACTURER_ID	0x7E
#define OPT3002_REG_DEVICE_ID		0x7F

#define OPT3002_MANUFACTURER_ID		0x5449

#define OPT3002_CFG_MODE_SHIFT		9
#define OPT3002_CFG_MODE_WIDTH		2
#define OPT3002_CFG_MODE_MASK		GENMASK(OPT3002_CFG_MODE_SHIFT + \
						(OPT3002_CFG_MODE_WIDTH - 1), \
						OPT3002_CFG_MODE_SHIFT)
#define OPT3002_CFG_MODE_SHUTDOWN	0x0
#define OPT3002_CFG_MODE_SINGLE_SHOT	0x1
#define OPT3002_CFG_MODE_CONTINUOUS	0x2

int opt3002_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	dev_info(&client->dev,
		 "In probe() for device: {name=%s, driver_data=%lu}\n",
		 id->name, id->driver_data);

	/*
	 * == TASK ==
	 * The header file linux/i2c.h declares i2c_smbus_read_word_swapped. Use
	 * this function to read the manufacturer ID register from the device
	 * and verify that it matches the expected OPT3002_MANUFACTURER_ID
	 * value. If the match fails, return errno -ENODEV.
	 */

	return 0;
}

int opt3002_remove(struct i2c_client *client)
{
	s32 cfg_reg, ret;

	dev_info(&client->dev, "In remove()\n");

	/*
	 * Ensure the device is in shutdown mode by writing the "mode" bitfield
	 * of the configuration register.
	 */
	cfg_reg = i2c_smbus_read_word_swapped(client, OPT3002_REG_CONFIGURATION);
	if (cfg_reg < 0) {
		dev_err(&client->dev, "Failed to read config register\n");
		return cfg_reg;
	}

	cfg_reg &= ~OPT3002_CFG_MODE_MASK;
	cfg_reg |= (OPT3002_CFG_MODE_SHUTDOWN << OPT3002_CFG_MODE_SHIFT);
	ret = i2c_smbus_write_word_swapped(client, OPT3002_REG_CONFIGURATION, cfg_reg);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to write config register\n");
		return ret;
	}

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
