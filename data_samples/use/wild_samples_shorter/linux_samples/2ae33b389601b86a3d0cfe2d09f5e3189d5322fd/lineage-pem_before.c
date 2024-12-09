	&sensor_dev_attr_in2_input.dev_attr.attr,
	&sensor_dev_attr_curr1_input.dev_attr.attr,
	&sensor_dev_attr_power1_input.dev_attr.attr,
};

static const struct attribute_group pem_input_group = {
	.attrs = pem_input_attributes,
	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&sensor_dev_attr_fan3_input.dev_attr.attr,
};

static const struct attribute_group pem_fan_group = {
	.attrs = pem_fan_attributes,