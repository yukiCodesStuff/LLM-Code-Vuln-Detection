	offset = be32_to_cpup(list);
	ret = mtd_read(mtd, offset, len, &retlen, eep);
	put_mtd_device(mtd);
	if (ret) {
		dev_err(dev->dev, "reading EEPROM from mtd %s failed: %i\n",
			part, ret);
		goto out_put_node;
	}

	if (retlen < len) {
		ret = -EINVAL;
		goto out_put_node;