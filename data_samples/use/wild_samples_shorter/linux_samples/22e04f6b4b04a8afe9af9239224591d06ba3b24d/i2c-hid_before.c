#include <linux/hid.h>
#include <linux/mutex.h>
#include <linux/acpi.h>

#include <linux/i2c/i2c-hid.h>

/* flags */
	return ret;
}

static int i2c_hid_hidinput_input_event(struct input_dev *dev,
		unsigned int type, unsigned int code, int value)
{
	struct hid_device *hid = input_get_drvdata(dev);
	struct hid_field *field;
	int offset;

	if (type == EV_FF)
		return input_ff_event(dev, type, code, value);

	if (type != EV_LED)
		return -1;

	offset = hidinput_find_field(hid, type, code, &field);

	if (offset == -1) {
		hid_warn(dev, "event field not found\n");
		return -1;
	}

	return hid_set_field(field, offset, value);
}

static struct hid_ll_driver i2c_hid_ll_driver = {
	.parse = i2c_hid_parse,
	.start = i2c_hid_start,
	.stop = i2c_hid_stop,
	.close = i2c_hid_close,
	.power = i2c_hid_power,
	.request = i2c_hid_request,
	.hidinput_input_event = i2c_hid_hidinput_input_event,
};

static int i2c_hid_init_irq(struct i2c_client *client)
{
	 * bytes 2-3 -> bcdVersion (has to be 1.00) */
	ret = i2c_hid_command(client, &hid_descr_cmd, ihid->hdesc_buffer, 4);

	i2c_hid_dbg(ihid, "%s, ihid->hdesc_buffer: %*ph\n",
			__func__, 4, ihid->hdesc_buffer);

	if (ret) {
		dev_err(&client->dev,
			"unable to fetch the size of HID descriptor (ret=%d)\n",
	params[1].integer.value = 1;
	params[2].type = ACPI_TYPE_INTEGER;
	params[2].integer.value = 1; /* HID function */
	params[3].type = ACPI_TYPE_INTEGER;
	params[3].integer.value = 0;

	if (ACPI_FAILURE(acpi_evaluate_object(handle, "_DSM", &input, &buf))) {
		dev_err(&client->dev, "device _DSM execution failed\n");
		return -ENODEV;
}
#endif

static int i2c_hid_probe(struct i2c_client *client,
			 const struct i2c_device_id *dev_id)
{
	int ret;
	if (!ihid)
		return -ENOMEM;

	if (!platform_data) {
		ret = i2c_hid_acpi_pdata(client, &ihid->pdata);
		if (ret) {
			dev_err(&client->dev,
				"HID register address not provided\n");
		.owner	= THIS_MODULE,
		.pm	= &i2c_hid_pm,
		.acpi_match_table = ACPI_PTR(i2c_hid_acpi_match),
	},

	.probe		= i2c_hid_probe,
	.remove		= i2c_hid_remove,