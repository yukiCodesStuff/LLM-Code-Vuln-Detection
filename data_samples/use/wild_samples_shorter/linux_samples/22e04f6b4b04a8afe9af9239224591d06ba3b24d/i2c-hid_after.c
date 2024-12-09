#include <linux/hid.h>
#include <linux/mutex.h>
#include <linux/acpi.h>
#include <linux/of.h>

#include <linux/i2c/i2c-hid.h>

/* flags */
	return ret;
}

static struct hid_ll_driver i2c_hid_ll_driver = {
	.parse = i2c_hid_parse,
	.start = i2c_hid_start,
	.stop = i2c_hid_stop,
	.close = i2c_hid_close,
	.power = i2c_hid_power,
	.request = i2c_hid_request,
};

static int i2c_hid_init_irq(struct i2c_client *client)
{
	 * bytes 2-3 -> bcdVersion (has to be 1.00) */
	ret = i2c_hid_command(client, &hid_descr_cmd, ihid->hdesc_buffer, 4);

	i2c_hid_dbg(ihid, "%s, ihid->hdesc_buffer: %4ph\n", __func__,
			ihid->hdesc_buffer);

	if (ret) {
		dev_err(&client->dev,
			"unable to fetch the size of HID descriptor (ret=%d)\n",
	params[1].integer.value = 1;
	params[2].type = ACPI_TYPE_INTEGER;
	params[2].integer.value = 1; /* HID function */
	params[3].type = ACPI_TYPE_PACKAGE;
	params[3].package.count = 0;
	params[3].package.elements = NULL;

	if (ACPI_FAILURE(acpi_evaluate_object(handle, "_DSM", &input, &buf))) {
		dev_err(&client->dev, "device _DSM execution failed\n");
		return -ENODEV;
}
#endif

#ifdef CONFIG_OF
static int i2c_hid_of_probe(struct i2c_client *client,
		struct i2c_hid_platform_data *pdata)
{
	struct device *dev = &client->dev;
	u32 val;
	int ret;

	ret = of_property_read_u32(dev->of_node, "hid-descr-addr", &val);
	if (ret) {
		dev_err(&client->dev, "HID register address not provided\n");
		return -ENODEV;
	}
	if (val >> 16) {
		dev_err(&client->dev, "Bad HID register address: 0x%08x\n",
			val);
		return -EINVAL;
	}
	pdata->hid_descriptor_address = val;

	return 0;
}

static const struct of_device_id i2c_hid_of_match[] = {
	{ .compatible = "hid-over-i2c" },
	{},
};
MODULE_DEVICE_TABLE(of, i2c_hid_of_match);
#else
static inline int i2c_hid_of_probe(struct i2c_client *client,
		struct i2c_hid_platform_data *pdata)
{
	return -ENODEV;
}
#endif

static int i2c_hid_probe(struct i2c_client *client,
			 const struct i2c_device_id *dev_id)
{
	int ret;
	if (!ihid)
		return -ENOMEM;

	if (client->dev.of_node) {
		ret = i2c_hid_of_probe(client, &ihid->pdata);
		if (ret)
			goto err;
	} else if (!platform_data) {
		ret = i2c_hid_acpi_pdata(client, &ihid->pdata);
		if (ret) {
			dev_err(&client->dev,
				"HID register address not provided\n");
		.owner	= THIS_MODULE,
		.pm	= &i2c_hid_pm,
		.acpi_match_table = ACPI_PTR(i2c_hid_acpi_match),
		.of_match_table = of_match_ptr(i2c_hid_of_match),
	},

	.probe		= i2c_hid_probe,
	.remove		= i2c_hid_remove,