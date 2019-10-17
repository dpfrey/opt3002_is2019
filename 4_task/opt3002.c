#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/delay.h>

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

#define OPT3002_CFG_CRF_SHIFT		7
#define OPT3002_CFG_CRF_WIDTH		1
#define OPT3002_CFG_CRF_MASK		BIT(OPT3002_CFG_CRF_SHIFT)

#define OPT3002_CFG_MODE_SHIFT		9
#define OPT3002_CFG_MODE_WIDTH		2
#define OPT3002_CFG_MODE_MASK		GENMASK(OPT3002_CFG_MODE_SHIFT + \
						(OPT3002_CFG_MODE_WIDTH - 1), \
						OPT3002_CFG_MODE_SHIFT)
#define OPT3002_CFG_MODE_SHUTDOWN	0x0
#define OPT3002_CFG_MODE_SINGLE_SHOT	0x1
#define OPT3002_CFG_MODE_CONTINUOUS	0x2

#define OPT3002_CFG_CONV_TIME_SHIFT	11
#define OPT3002_CFG_CONV_TIME_WIDTH	1
#define OPT3002_CFG_CONV_TIME_MASK	BIT(OPT3002_CFG_CONV_TIME_SHIFT)
#define OPT3002_CFG_CONV_TIME_100MS	0x0
#define OPT3002_CFG_CONV_TIME_800MS	0x1

#define OPT3002_CFG_RN_SHIFT		12
#define OPT3002_CFG_RN_WIDTH		4
#define OPT3002_CFG_RN_MASK		GENMASK(OPT3002_CFG_RN_SHIFT + \
						(OPT3002_CFG_RN_WIDTH - 1), \
						OPT3002_CFG_RN_SHIFT)
#define OPT3002_CFG_RN_AUTO		0xC


/*
 * == TASK ==
 * Read through the following function and understand the code that is required
 * to perform a reading.
 */
s32 opt3002_perform_reading(struct i2c_client *client)
{
	u8 exp;
	u16 frac;
	s32 ret;
	unsigned int retries;

	/*
	 * Configure the device for a single 800 ms sample with automatic range
	 * selection.
	 */
	u16 cfg_reg = (
		(OPT3002_CFG_MODE_SINGLE_SHOT << OPT3002_CFG_MODE_SHIFT) |
		(OPT3002_CFG_CONV_TIME_800MS << OPT3002_CFG_CONV_TIME_SHIFT) |
		(OPT3002_CFG_RN_AUTO << OPT3002_CFG_RN_SHIFT));
	ret = i2c_smbus_write_word_swapped(client, OPT3002_REG_CONFIGURATION, cfg_reg);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to write config register\n");
		return ret;
	}

	for (retries = 0; retries < 5; retries++) {
		msleep(800);
		ret = i2c_smbus_read_word_swapped(client, OPT3002_REG_CONFIGURATION);
		if (ret < 0)
			continue;
		if ((ret & OPT3002_CFG_CRF_MASK) != 0) {
			break;
		}
	}

	if (retries >= 5)
		return -EBUSY;

	ret = i2c_smbus_read_word_swapped(client, OPT3002_REG_RESULT);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to read result register\n");
		return ret;
	}

	/* See datasheet page 20: 6/5 comes from 1.2 constant */
	exp = (ret >> 12);
	frac = (ret & 0x0FFF);

	return ((1 << exp) * 6 * frac) / 5;
}

int opt3002_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	s32 mfg_id;
	dev_info(&client->dev,
		 "In probe() for device: {name=%s, driver_data=%lu}\n",
		 id->name, id->driver_data);

	mfg_id = i2c_smbus_read_word_swapped(client, OPT3002_REG_MANUFACTURER_ID);
	if (mfg_id < 0) {
		dev_err(&client->dev, "Failed to read manufacturer ID\n");
		return mfg_id;
	}

	if (mfg_id != OPT3002_MANUFACTURER_ID) {
		dev_err(&client->dev,
			"Expected manufacturer ID 0x%04X, but read 0x%04X\n",
			OPT3002_MANUFACTURER_ID, mfg_id);
		return -ENODEV;
	}

	/*
	 * == TASK ==
	 * Use dev_info to print a single reading from the opt3002 light sensor
	 * and observe that the value is printed when the module is loaded.
	 */

	return 0;
}

int opt3002_remove(struct i2c_client *client)
{
	s32 cfg_reg, ret;

	dev_info(&client->dev, "In remove()\n");

	/*
	 * Ensure the device is in shutdown mode by writing the "mode" bitfield
	 * of the configuration register. We use i2c_smbus_read_word_swapped
	 * instead of i2c_smbus_read_word_data because the device provides the most
	 * significant byte first.
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
