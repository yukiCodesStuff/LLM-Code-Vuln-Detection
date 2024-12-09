struct hid_sensor_common {
	struct hid_sensor_hub_device *hsdev;
	struct platform_device *pdev;
	unsigned usage_id;
	bool data_ready;
	struct hid_sensor_hub_attribute_info poll;
	struct hid_sensor_hub_attribute_info report_state;
	struct hid_sensor_hub_attribute_info power_state;
	struct hid_sensor_hub_attribute_info sensitivity;
};

/* Convert from hid unit expo to regular exponent */
static inline int hid_sensor_convert_exponent(int unit_expo)
{
	if (unit_expo < 0x08)
		return unit_expo;
	else if (unit_expo <= 0x0f)
		return -(0x0f-unit_expo+1);
	else
		return 0;
}