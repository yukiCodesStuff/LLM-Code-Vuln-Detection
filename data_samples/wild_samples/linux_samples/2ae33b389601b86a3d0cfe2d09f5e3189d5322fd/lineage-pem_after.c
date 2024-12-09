static struct attribute *pem_input_attributes[] = {
	&sensor_dev_attr_in2_input.dev_attr.attr,
	&sensor_dev_attr_curr1_input.dev_attr.attr,
	&sensor_dev_attr_power1_input.dev_attr.attr,
	NULL
};

static const struct attribute_group pem_input_group = {
	.attrs = pem_input_attributes,
};

static struct attribute *pem_fan_attributes[] = {
	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&sensor_dev_attr_fan3_input.dev_attr.attr,
	NULL
};

static const struct attribute_group pem_fan_group = {
	.attrs = pem_fan_attributes,
};

static int pem_probe(struct i2c_client *client,
		     const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = client->adapter;
	struct pem_data *data;
	int ret;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BLOCK_DATA
				     | I2C_FUNC_SMBUS_WRITE_BYTE))
		return -ENODEV;

	data = devm_kzalloc(&client->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	i2c_set_clientdata(client, data);
	mutex_init(&data->update_lock);

	/*
	 * We use the next two commands to determine if the device is really
	 * there.
	 */
	ret = pem_read_block(client, PEM_READ_FIRMWARE_REV,
			     data->firmware_rev, sizeof(data->firmware_rev));
	if (ret < 0)
		return ret;

	ret = i2c_smbus_write_byte(client, PEM_CLEAR_INFO_FLAGS);
	if (ret < 0)
		return ret;

	dev_info(&client->dev, "Firmware revision %d.%d.%d\n",
		 data->firmware_rev[0], data->firmware_rev[1],
		 data->firmware_rev[2]);

	/* Register sysfs hooks */
	ret = sysfs_create_group(&client->dev.kobj, &pem_group);
	if (ret)
		return ret;

	/*
	 * Check if input readings are supported.
	 * This is the case if we can read input data,
	 * and if the returned data is not all zeros.
	 * Note that input alarms are always supported.
	 */
	ret = pem_read_block(client, PEM_READ_INPUT_STRING,
			     data->input_string,
			     sizeof(data->input_string) - 1);
	if (!ret && (data->input_string[0] || data->input_string[1] ||
		     data->input_string[2]))
		data->input_length = sizeof(data->input_string) - 1;
	else if (ret < 0) {
		/* Input string is one byte longer for some devices */
		ret = pem_read_block(client, PEM_READ_INPUT_STRING,
				    data->input_string,
				    sizeof(data->input_string));
		if (!ret && (data->input_string[0] || data->input_string[1] ||
			    data->input_string[2] || data->input_string[3]))
			data->input_length = sizeof(data->input_string);
	}
	ret = 0;
	if (data->input_length) {
		ret = sysfs_create_group(&client->dev.kobj, &pem_input_group);
		if (ret)
			goto out_remove_groups;
	}

	/*
	 * Check if fan speed readings are supported.
	 * This is the case if we can read fan speed data,
	 * and if the returned data is not all zeros.
	 * Note that the fan alarm is always supported.
	 */
	ret = pem_read_block(client, PEM_READ_FAN_SPEED,
			     data->fan_speed,
			     sizeof(data->fan_speed));
	if (!ret && (data->fan_speed[0] || data->fan_speed[1] ||
		     data->fan_speed[2] || data->fan_speed[3])) {
		data->fans_supported = true;
		ret = sysfs_create_group(&client->dev.kobj, &pem_fan_group);
		if (ret)
			goto out_remove_groups;
	}

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		ret = PTR_ERR(data->hwmon_dev);
		goto out_remove_groups;
	}

	return 0;

out_remove_groups:
	sysfs_remove_group(&client->dev.kobj, &pem_input_group);
	sysfs_remove_group(&client->dev.kobj, &pem_fan_group);
	sysfs_remove_group(&client->dev.kobj, &pem_group);
	return ret;
}

static int pem_remove(struct i2c_client *client)
{
	struct pem_data *data = i2c_get_clientdata(client);

	hwmon_device_unregister(data->hwmon_dev);

	sysfs_remove_group(&client->dev.kobj, &pem_input_group);
	sysfs_remove_group(&client->dev.kobj, &pem_fan_group);
	sysfs_remove_group(&client->dev.kobj, &pem_group);

	return 0;
}

static const struct i2c_device_id pem_id[] = {
	{"lineage_pem", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, pem_id);

static struct i2c_driver pem_driver = {
	.driver = {
		   .name = "lineage_pem",
		   },
	.probe = pem_probe,
	.remove = pem_remove,
	.id_table = pem_id,
};

module_i2c_driver(pem_driver);

MODULE_AUTHOR("Guenter Roeck <linux@roeck-us.net>");
MODULE_DESCRIPTION("Lineage CPL PEM hardware monitoring driver");
MODULE_LICENSE("GPL");
static struct attribute *pem_fan_attributes[] = {
	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&sensor_dev_attr_fan3_input.dev_attr.attr,
	NULL
};

static const struct attribute_group pem_fan_group = {
	.attrs = pem_fan_attributes,
};

static int pem_probe(struct i2c_client *client,
		     const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = client->adapter;
	struct pem_data *data;
	int ret;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BLOCK_DATA
				     | I2C_FUNC_SMBUS_WRITE_BYTE))
		return -ENODEV;

	data = devm_kzalloc(&client->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	i2c_set_clientdata(client, data);
	mutex_init(&data->update_lock);

	/*
	 * We use the next two commands to determine if the device is really
	 * there.
	 */
	ret = pem_read_block(client, PEM_READ_FIRMWARE_REV,
			     data->firmware_rev, sizeof(data->firmware_rev));
	if (ret < 0)
		return ret;

	ret = i2c_smbus_write_byte(client, PEM_CLEAR_INFO_FLAGS);
	if (ret < 0)
		return ret;

	dev_info(&client->dev, "Firmware revision %d.%d.%d\n",
		 data->firmware_rev[0], data->firmware_rev[1],
		 data->firmware_rev[2]);

	/* Register sysfs hooks */
	ret = sysfs_create_group(&client->dev.kobj, &pem_group);
	if (ret)
		return ret;

	/*
	 * Check if input readings are supported.
	 * This is the case if we can read input data,
	 * and if the returned data is not all zeros.
	 * Note that input alarms are always supported.
	 */
	ret = pem_read_block(client, PEM_READ_INPUT_STRING,
			     data->input_string,
			     sizeof(data->input_string) - 1);
	if (!ret && (data->input_string[0] || data->input_string[1] ||
		     data->input_string[2]))
		data->input_length = sizeof(data->input_string) - 1;
	else if (ret < 0) {
		/* Input string is one byte longer for some devices */
		ret = pem_read_block(client, PEM_READ_INPUT_STRING,
				    data->input_string,
				    sizeof(data->input_string));
		if (!ret && (data->input_string[0] || data->input_string[1] ||
			    data->input_string[2] || data->input_string[3]))
			data->input_length = sizeof(data->input_string);
	}
	ret = 0;
	if (data->input_length) {
		ret = sysfs_create_group(&client->dev.kobj, &pem_input_group);
		if (ret)
			goto out_remove_groups;
	}

	/*
	 * Check if fan speed readings are supported.
	 * This is the case if we can read fan speed data,
	 * and if the returned data is not all zeros.
	 * Note that the fan alarm is always supported.
	 */
	ret = pem_read_block(client, PEM_READ_FAN_SPEED,
			     data->fan_speed,
			     sizeof(data->fan_speed));
	if (!ret && (data->fan_speed[0] || data->fan_speed[1] ||
		     data->fan_speed[2] || data->fan_speed[3])) {
		data->fans_supported = true;
		ret = sysfs_create_group(&client->dev.kobj, &pem_fan_group);
		if (ret)
			goto out_remove_groups;
	}

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		ret = PTR_ERR(data->hwmon_dev);
		goto out_remove_groups;
	}

	return 0;

out_remove_groups:
	sysfs_remove_group(&client->dev.kobj, &pem_input_group);
	sysfs_remove_group(&client->dev.kobj, &pem_fan_group);
	sysfs_remove_group(&client->dev.kobj, &pem_group);
	return ret;
}

static int pem_remove(struct i2c_client *client)
{
	struct pem_data *data = i2c_get_clientdata(client);

	hwmon_device_unregister(data->hwmon_dev);

	sysfs_remove_group(&client->dev.kobj, &pem_input_group);
	sysfs_remove_group(&client->dev.kobj, &pem_fan_group);
	sysfs_remove_group(&client->dev.kobj, &pem_group);

	return 0;
}

static const struct i2c_device_id pem_id[] = {
	{"lineage_pem", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, pem_id);

static struct i2c_driver pem_driver = {
	.driver = {
		   .name = "lineage_pem",
		   },
	.probe = pem_probe,
	.remove = pem_remove,
	.id_table = pem_id,
};

module_i2c_driver(pem_driver);

MODULE_AUTHOR("Guenter Roeck <linux@roeck-us.net>");
MODULE_DESCRIPTION("Lineage CPL PEM hardware monitoring driver");
MODULE_LICENSE("GPL");