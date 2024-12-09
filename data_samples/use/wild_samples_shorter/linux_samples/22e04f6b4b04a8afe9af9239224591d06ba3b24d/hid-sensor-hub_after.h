	struct hid_sensor_hub_attribute_info sensitivity;
};

/* Convert from hid unit expo to regular exponent */
static inline int hid_sensor_convert_exponent(int unit_expo)
{
	if (unit_expo < 0x08)
		return unit_expo;