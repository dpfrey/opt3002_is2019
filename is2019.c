#include <linux/i2c.h>
#include <linux/i2c/pca954x.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>

static struct pca954x_platform_mode pca9546_adap_modes[] = {
	{.adap_id=5, .deselect_on_exit=1, .class=0},
	{.adap_id=6, .deselect_on_exit=1, .class=0},
	{.adap_id=7, .deselect_on_exit=1, .class=0},
	{.adap_id=8, .deselect_on_exit=1, .class=0},
};

static struct pca954x_platform_data pca9546_platform_data = {
	.modes=pca9546_adap_modes,
	.num_modes=ARRAY_SIZE(pca9546_adap_modes),
};

static const struct i2c_board_info pca9546_device_info = {
	I2C_BOARD_INFO("pca9546", 0x71),
	.platform_data = &pca9546_platform_data,
};

static const struct i2c_board_info opt3002_device_info = {
	I2C_BOARD_INFO("opt3002", 0x44),
};

static struct i2c_client *i2c_switch;
static struct i2c_client *light_sensor;

int __init is2019_init(void)
{
	struct i2c_adapter *i2c_adapter_primary;
	struct i2c_adapter *i2c_adapter_light_sensor;

	i2c_adapter_primary = i2c_get_adapter(4);
	if (!i2c_adapter_primary) {
		pr_err("Failed to get primary I2C adapter\n");
		return -ENODEV;
	}

	i2c_switch = i2c_new_device(i2c_adapter_primary, &pca9546_device_info);
	i2c_put_adapter(i2c_adapter_primary);
	if (!i2c_switch) {
		pr_err("i2c_new_device failed for pca9546\n");
		return -ENODEV;
	}

	i2c_adapter_light_sensor = i2c_get_adapter(7);
	if (!i2c_adapter_light_sensor) {
		pr_err("Failed to get light sensor I2C adapter\n");
		return -ENODEV;
	}

	light_sensor = i2c_new_device(i2c_adapter_light_sensor, &opt3002_device_info);
	i2c_put_adapter(i2c_adapter_light_sensor);
	if (!light_sensor)
	{
		pr_err("i2c_new_device failed for opt3002\n");
		return -ENODEV;
	}

	return 0;
}
module_init(is2019_init);

void __exit is2019_exit(void)
{
	i2c_unregister_device(light_sensor);
	i2c_unregister_device(i2c_switch);
}
module_exit(is2019_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("David Frey <dfrey@sierrawireless.com>");
MODULE_DESCRIPTION("Minimal replacement for mangOH Yellow module");
