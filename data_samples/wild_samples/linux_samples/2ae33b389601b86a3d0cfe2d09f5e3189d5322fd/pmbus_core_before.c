{
	if (data->num_attributes >= data->max_attributes - 1) {
		data->max_attributes += PMBUS_ATTR_ALLOC_SIZE;
		data->group.attrs = krealloc(data->group.attrs,
					     sizeof(struct attribute *) *
					     data->max_attributes, GFP_KERNEL);
		if (data->group.attrs == NULL)
			return -ENOMEM;
	}

	data->group.attrs[data->num_attributes++] = attr;
	data->group.attrs[data->num_attributes] = NULL;
	return 0;
}