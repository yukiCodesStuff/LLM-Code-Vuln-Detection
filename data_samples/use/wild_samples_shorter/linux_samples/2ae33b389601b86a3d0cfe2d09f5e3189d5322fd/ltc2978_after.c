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
		ret = pmbus_read_word_data(client, page,
					   LTC2978_MFR_TEMPERATURE_PEAK);
		if (ret >= 0) {
			if (lin11_to_val(ret)
			    > lin11_to_val(data->temp_max[page]))
				data->temp_max[page] = ret;
			ret = data->temp_max[page];
		}
		break;
	case PMBUS_VIRT_RESET_VOUT_HISTORY:
	case PMBUS_VIRT_RESET_VIN_HISTORY:
		ret = pmbus_read_word_data(client, page,
					   LTC3880_MFR_TEMPERATURE2_PEAK);
		if (ret >= 0) {
			if (lin11_to_val(ret) > lin11_to_val(data->temp2_max))
				data->temp2_max = ret;
			ret = data->temp2_max;
		}
		break;
	case PMBUS_VIRT_READ_VIN_MIN:
	case PMBUS_VIRT_READ_VOUT_MIN:

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
		for (i = 1; i < 8; i++) {
			info->func[i] = PMBUS_HAVE_VOUT
			  | PMBUS_HAVE_STATUS_VOUT;
		}
		break;
	case ltc3880:
		info->read_word_data = ltc3880_read_word_data;
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
