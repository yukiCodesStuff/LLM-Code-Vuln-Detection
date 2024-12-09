struct ltc2978_data {
	enum chips id;
	int vin_min, vin_max;
	int temp_min, temp_max[2];
	int vout_min[8], vout_max[8];
	int iout_max[2];
	int temp2_max;
	struct pmbus_driver_info info;
};

#define to_ltc2978_data(x)  container_of(x, struct ltc2978_data, info)

static inline int lin11_to_val(int data)
{
	s16 e = ((s16)data) >> 11;
	s32 m = (((s16)(data << 5)) >> 5);

	/*
	 * mantissa is 10 bit + sign, exponent adds up to 15 bit.
	 * Add 6 bit to exponent for maximum accuracy (10 + 15 + 6 = 31).
	 */
	e += 6;
	return (e < 0 ? m >> -e : m << e);
}
		if (ret >= 0) {
			/*
			 * VOUT is 16 bit unsigned with fixed exponent,
			 * so we can compare it directly
			 */
			if (ret > data->vout_max[page])
				data->vout_max[page] = ret;
			ret = data->vout_max[page];
		}
		break;
	case PMBUS_VIRT_READ_TEMP_MAX:
		ret = pmbus_read_word_data(client, page,
					   LTC2978_MFR_TEMPERATURE_PEAK);
		if (ret >= 0) {
			if (lin11_to_val(ret)
			    > lin11_to_val(data->temp_max[page]))
				data->temp_max[page] = ret;
			ret = data->temp_max[page];
		}
		if (ret >= 0) {
			if (lin11_to_val(ret)
			    > lin11_to_val(data->iout_max[page]))
				data->iout_max[page] = ret;
			ret = data->iout_max[page];
		}
		break;
	case PMBUS_VIRT_READ_TEMP2_MAX:
		ret = pmbus_read_word_data(client, page,
					   LTC3880_MFR_TEMPERATURE2_PEAK);
		if (ret >= 0) {
			if (lin11_to_val(ret) > lin11_to_val(data->temp2_max))
				data->temp2_max = ret;
			ret = data->temp2_max;
		}
	switch (reg) {
	case PMBUS_VIRT_RESET_IOUT_HISTORY:
		data->iout_max[page] = 0x7c00;
		ret = ltc2978_clear_peaks(client, page, data->id);
		break;
	case PMBUS_VIRT_RESET_TEMP2_HISTORY:
		data->temp2_max = 0x7c00;
		ret = ltc2978_clear_peaks(client, page, data->id);
		break;
	case PMBUS_VIRT_RESET_VOUT_HISTORY:
		data->vout_min[page] = 0xffff;
		data->vout_max[page] = 0;
		ret = ltc2978_clear_peaks(client, page, data->id);
		break;
	case PMBUS_VIRT_RESET_VIN_HISTORY:
		data->vin_min = 0x7bff;
		data->vin_max = 0x7c00;
		ret = ltc2978_clear_peaks(client, page, data->id);
		break;
	case PMBUS_VIRT_RESET_TEMP_HISTORY:
		data->temp_min = 0x7bff;
		data->temp_max[page] = 0x7c00;
		ret = ltc2978_clear_peaks(client, page, data->id);
		break;
	default:
		ret = -ENODATA;
		break;
	}
	switch (reg) {
	case PMBUS_VIRT_RESET_IOUT_HISTORY:
		data->iout_max[page] = 0x7c00;
		ret = ltc2978_clear_peaks(client, page, data->id);
		break;
	case PMBUS_VIRT_RESET_TEMP2_HISTORY:
		data->temp2_max = 0x7c00;
		ret = ltc2978_clear_peaks(client, page, data->id);
		break;
	case PMBUS_VIRT_RESET_VOUT_HISTORY:
		data->vout_min[page] = 0xffff;
		data->vout_max[page] = 0;
		ret = ltc2978_clear_peaks(client, page, data->id);
		break;
	case PMBUS_VIRT_RESET_VIN_HISTORY:
		data->vin_min = 0x7bff;
		data->vin_max = 0x7c00;
		ret = ltc2978_clear_peaks(client, page, data->id);
		break;
	case PMBUS_VIRT_RESET_TEMP_HISTORY:
		data->temp_min = 0x7bff;
		data->temp_max[page] = 0x7c00;
		ret = ltc2978_clear_peaks(client, page, data->id);
		break;
	default:
		ret = -ENODATA;
		break;
	}
	return ret;
}

static const struct i2c_device_id ltc2978_id[] = {
	{"ltc2978", ltc2978},
	{"ltc3880", ltc3880},
	{}
};
MODULE_DEVICE_TABLE(i2c, ltc2978_id);

static int ltc2978_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	int chip_id, i;
	struct ltc2978_data *data;
	struct pmbus_driver_info *info;

	if (!i2c_check_functionality(client->adapter,
				     I2C_FUNC_SMBUS_READ_WORD_DATA))
		return -ENODEV;

	data = devm_kzalloc(&client->dev, sizeof(struct ltc2978_data),
			    GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	chip_id = i2c_smbus_read_word_data(client, LTC2978_MFR_SPECIAL_ID);
	if (chip_id < 0)
		return chip_id;

	if (chip_id == LTC2978_ID_REV1 || chip_id == LTC2978_ID_REV2) {
		data->id = ltc2978;
	} else if ((chip_id & LTC3880_ID_MASK) == LTC3880_ID) {
		data->id = ltc3880;
	} else {
		dev_err(&client->dev, "Unsupported chip ID 0x%x\n", chip_id);
		return -ENODEV;
	}
	if (data->id != id->driver_data)
		dev_warn(&client->dev,
			 "Device mismatch: Configured %s, detected %s\n",
			 id->name,
			 ltc2978_id[data->id].name);

	info = &data->info;
	info->write_word_data = ltc2978_write_word_data;

	data->vin_min = 0x7bff;
	data->vin_max = 0x7c00;
	data->temp_min = 0x7bff;
	for (i = 0; i < ARRAY_SIZE(data->temp_max); i++)
		data->temp_max[i] = 0x7c00;
	data->temp2_max = 0x7c00;

	switch (data->id) {
	case ltc2978:
		info->read_word_data = ltc2978_read_word_data;
		info->pages = 8;
		info->func[0] = PMBUS_HAVE_VIN | PMBUS_HAVE_STATUS_INPUT
		  | PMBUS_HAVE_VOUT | PMBUS_HAVE_STATUS_VOUT
		  | PMBUS_HAVE_TEMP | PMBUS_HAVE_STATUS_TEMP;
		for (i = 1; i < 8; i++) {
			info->func[i] = PMBUS_HAVE_VOUT
			  | PMBUS_HAVE_STATUS_VOUT;
		}
		break;
	case ltc3880:
		info->read_word_data = ltc3880_read_word_data;
		info->pages = 2;
		info->func[0] = PMBUS_HAVE_VIN | PMBUS_HAVE_IIN
		  | PMBUS_HAVE_STATUS_INPUT
		  | PMBUS_HAVE_VOUT | PMBUS_HAVE_STATUS_VOUT
		  | PMBUS_HAVE_IOUT | PMBUS_HAVE_STATUS_IOUT
		  | PMBUS_HAVE_POUT | PMBUS_HAVE_TEMP
		  | PMBUS_HAVE_TEMP2 | PMBUS_HAVE_STATUS_TEMP;
		info->func[1] = PMBUS_HAVE_VOUT | PMBUS_HAVE_STATUS_VOUT
		  | PMBUS_HAVE_IOUT | PMBUS_HAVE_STATUS_IOUT
		  | PMBUS_HAVE_POUT
		  | PMBUS_HAVE_TEMP | PMBUS_HAVE_STATUS_TEMP;
		data->iout_max[0] = 0x7c00;
		data->iout_max[1] = 0x7c00;
		break;
	default:
		return -ENODEV;
	}
	for (i = 0; i < info->pages; i++)
		data->vout_min[i] = 0xffff;

	return pmbus_do_probe(client, id, info);
}
	} else {
		dev_err(&client->dev, "Unsupported chip ID 0x%x\n", chip_id);
		return -ENODEV;
	}
	if (data->id != id->driver_data)
		dev_warn(&client->dev,
			 "Device mismatch: Configured %s, detected %s\n",
			 id->name,
			 ltc2978_id[data->id].name);

	info = &data->info;
	info->write_word_data = ltc2978_write_word_data;

	data->vin_min = 0x7bff;
	data->vin_max = 0x7c00;
	data->temp_min = 0x7bff;
	for (i = 0; i < ARRAY_SIZE(data->temp_max); i++)
		data->temp_max[i] = 0x7c00;
	data->temp2_max = 0x7c00;

	switch (data->id) {
	case ltc2978:
		info->read_word_data = ltc2978_read_word_data;
		info->pages = 8;
		info->func[0] = PMBUS_HAVE_VIN | PMBUS_HAVE_STATUS_INPUT
		  | PMBUS_HAVE_VOUT | PMBUS_HAVE_STATUS_VOUT
		  | PMBUS_HAVE_TEMP | PMBUS_HAVE_STATUS_TEMP;
		for (i = 1; i < 8; i++) {
			info->func[i] = PMBUS_HAVE_VOUT
			  | PMBUS_HAVE_STATUS_VOUT;
		}
		break;
	case ltc3880:
		info->read_word_data = ltc3880_read_word_data;
		info->pages = 2;
		info->func[0] = PMBUS_HAVE_VIN | PMBUS_HAVE_IIN
		  | PMBUS_HAVE_STATUS_INPUT
		  | PMBUS_HAVE_VOUT | PMBUS_HAVE_STATUS_VOUT
		  | PMBUS_HAVE_IOUT | PMBUS_HAVE_STATUS_IOUT
		  | PMBUS_HAVE_POUT | PMBUS_HAVE_TEMP
		  | PMBUS_HAVE_TEMP2 | PMBUS_HAVE_STATUS_TEMP;
		info->func[1] = PMBUS_HAVE_VOUT | PMBUS_HAVE_STATUS_VOUT
		  | PMBUS_HAVE_IOUT | PMBUS_HAVE_STATUS_IOUT
		  | PMBUS_HAVE_POUT
		  | PMBUS_HAVE_TEMP | PMBUS_HAVE_STATUS_TEMP;
		data->iout_max[0] = 0x7c00;
		data->iout_max[1] = 0x7c00;
		break;
	default:
		return -ENODEV;
	}
		for (i = 1; i < 8; i++) {
			info->func[i] = PMBUS_HAVE_VOUT
			  | PMBUS_HAVE_STATUS_VOUT;
		}
		break;
	case ltc3880:
		info->read_word_data = ltc3880_read_word_data;
		info->pages = 2;
		info->func[0] = PMBUS_HAVE_VIN | PMBUS_HAVE_IIN
		  | PMBUS_HAVE_STATUS_INPUT
		  | PMBUS_HAVE_VOUT | PMBUS_HAVE_STATUS_VOUT
		  | PMBUS_HAVE_IOUT | PMBUS_HAVE_STATUS_IOUT
		  | PMBUS_HAVE_POUT | PMBUS_HAVE_TEMP
		  | PMBUS_HAVE_TEMP2 | PMBUS_HAVE_STATUS_TEMP;
		info->func[1] = PMBUS_HAVE_VOUT | PMBUS_HAVE_STATUS_VOUT
		  | PMBUS_HAVE_IOUT | PMBUS_HAVE_STATUS_IOUT
		  | PMBUS_HAVE_POUT
		  | PMBUS_HAVE_TEMP | PMBUS_HAVE_STATUS_TEMP;
		data->iout_max[0] = 0x7c00;
		data->iout_max[1] = 0x7c00;
		break;
	default:
		return -ENODEV;
	}
	for (i = 0; i < info->pages; i++)
		data->vout_min[i] = 0xffff;

	return pmbus_do_probe(client, id, info);
}

/* This is the driver that will be inserted */
static struct i2c_driver ltc2978_driver = {
	.driver = {
		   .name = "ltc2978",
		   },
	.probe = ltc2978_probe,
	.remove = pmbus_do_remove,
	.id_table = ltc2978_id,
};

module_i2c_driver(ltc2978_driver);

MODULE_AUTHOR("Guenter Roeck");
MODULE_DESCRIPTION("PMBus driver for LTC2978 and LTC3880");
MODULE_LICENSE("GPL");
		for (i = 1; i < 8; i++) {
			info->func[i] = PMBUS_HAVE_VOUT
			  | PMBUS_HAVE_STATUS_VOUT;
		}
		break;
	case ltc3880:
		info->read_word_data = ltc3880_read_word_data;
		info->pages = 2;
		info->func[0] = PMBUS_HAVE_VIN | PMBUS_HAVE_IIN
		  | PMBUS_HAVE_STATUS_INPUT
		  | PMBUS_HAVE_VOUT | PMBUS_HAVE_STATUS_VOUT
		  | PMBUS_HAVE_IOUT | PMBUS_HAVE_STATUS_IOUT
		  | PMBUS_HAVE_POUT | PMBUS_HAVE_TEMP
		  | PMBUS_HAVE_TEMP2 | PMBUS_HAVE_STATUS_TEMP;
		info->func[1] = PMBUS_HAVE_VOUT | PMBUS_HAVE_STATUS_VOUT
		  | PMBUS_HAVE_IOUT | PMBUS_HAVE_STATUS_IOUT
		  | PMBUS_HAVE_POUT
		  | PMBUS_HAVE_TEMP | PMBUS_HAVE_STATUS_TEMP;
		data->iout_max[0] = 0x7c00;
		data->iout_max[1] = 0x7c00;
		break;
	default:
		return -ENODEV;
	}
	for (i = 0; i < info->pages; i++)
		data->vout_min[i] = 0xffff;

	return pmbus_do_probe(client, id, info);
}

/* This is the driver that will be inserted */
static struct i2c_driver ltc2978_driver = {
	.driver = {
		   .name = "ltc2978",
		   },
	.probe = ltc2978_probe,
	.remove = pmbus_do_remove,
	.id_table = ltc2978_id,
};

module_i2c_driver(ltc2978_driver);

MODULE_AUTHOR("Guenter Roeck");
MODULE_DESCRIPTION("PMBus driver for LTC2978 and LTC3880");
MODULE_LICENSE("GPL");