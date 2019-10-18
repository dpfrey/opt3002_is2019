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
 * Here we introduce a struct to hold device state
 */
struct opt3002 {
	struct i2c_client *client;
	struct device *dev;
};

/*
 * Notice that the parameter to this function has changed from "struct
 * i2c_client *client" to our new device type.
 */
s32 opt3002_perform_reading(struct opt3002 *opt3002)
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
	ret = i2c_smbus_write_word_swapped(opt3002->client,
					   OPT3002_REG_CONFIGURATION, cfg_reg);
	if (ret < 0) {
		dev_err(opt3002->dev, "Failed to write config register\n");
		return ret;
	}

	for (retries = 0; retries < 5; retries++) {
		msleep(800);
		ret = i2c_smbus_read_word_swapped(opt3002->client, OPT3002_REG_CONFIGURATION);
		if (ret < 0)
			continue;
		if ((ret & OPT3002_CFG_CRF_MASK) != 0) {
			break;
		}
	}

	if (retries >= 5)
		return -EBUSY;

	ret = i2c_smbus_read_word_swapped(opt3002->client, OPT3002_REG_RESULT);
	if (ret < 0) {
		dev_err(opt3002->dev, "Failed to read result register\n");
		return ret;
	}

	/* See datasheet page 20: 6/5 comes from 1.2 constant */
	exp = (ret >> 12);
	frac = (ret & 0x0FFF);

	return ((1 << exp) * 6 * frac) / 5;
}

/*
 * The DEVICE_ATTR_RO convenience macro is used below to define the necessary
 * struct that describes the read-only (RO) attribute named "optical_power". The
 * function optical_power_show must have this exact name because the
 * DEVICE_ATTR_RO(optical_power) macro initializes a struct named
 * dev_attr_optical_power with a "show" function pointer member initialized to
 * optical_power_show. This function is responsible for writing buf with a
 * string representation of the optical power and returning the length of the
 * data placed in buf.
 */
static ssize_t optical_power_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	ssize_t len = 0;
	/* struct opt3002 *opt3002 = dev_get_drvdata(dev); */

	/*
	 * == TASK ==
	 * Get a reading from the sensor and use sprintf() to put a string
	 * version into buf.
	 */

	return len;
}
static DEVICE_ATTR_RO(optical_power);

int opt3002_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	s32 mfg_id;
	struct opt3002 *opt3002;

	dev_info(&client->dev,
		 "In probe() for device: {name=%s, driver_data=%lu}\n",
		 id->name, id->driver_data);

	/*
	 * Here we dynamically allocate memory for our opt3002 data structure
	 * and initialize the members of the struct.
	 */
	opt3002 = devm_kzalloc(&client->dev, sizeof(*opt3002), GFP_KERNEL);
	if (!opt3002)
		return -ENOMEM;

	opt3002->client = client;
	opt3002->dev = &client->dev;
	/*
	 * dev_set_drvdata is an important function that allows a driver to set
	 * one data pointer to contain driver specific data. Doing so allows
	 * functions to retrieve the device data using the corresponding
	 * dev_get_drvdata function. See the example above in
	 * optical_power_show().
	 */
	dev_set_drvdata(opt3002->dev, opt3002);

	mfg_id = i2c_smbus_read_word_swapped(client, OPT3002_REG_MANUFACTURER_ID);
	if (mfg_id < 0) {
		dev_err(opt3002->dev, "Failed to read manufacturer ID\n");
		return mfg_id;
	}

	if (mfg_id != OPT3002_MANUFACTURER_ID) {
		dev_err(opt3002->dev,
			"Expected manufacturer ID 0x%04X, but read 0x%04X\n",
			OPT3002_MANUFACTURER_ID, mfg_id);
		return -ENODEV;
	}

	/*
	 * Notice that we have removed the test code that called
	 * opt3002_perform_reading() since we will be exposing this via sysfs.
	 */

	/* Create the sysfs file for reading the light sensor */
	return device_create_file(opt3002->dev, &dev_attr_optical_power);
}

int opt3002_remove(struct i2c_client *client)
{
	s32 cfg_reg, ret;

	dev_info(&client->dev, "In remove()\n");

	/*
	 * == TASK ==
	 * The final thing that the probe function does is create the sysfs file
	 * using device_create_file. Add a function call here to remove the
	 * file.
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
