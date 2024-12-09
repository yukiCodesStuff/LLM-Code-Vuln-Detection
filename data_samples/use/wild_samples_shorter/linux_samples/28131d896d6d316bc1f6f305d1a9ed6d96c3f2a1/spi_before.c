	spi->bits_per_word = 32;

	glue->reg = devm_regulator_get(&spi->dev, "vwlan");
	if (PTR_ERR(glue->reg) == -EPROBE_DEFER)
		return -EPROBE_DEFER;
	if (IS_ERR(glue->reg)) {
		dev_err(glue->dev, "can't get regulator\n");
		return PTR_ERR(glue->reg);
	}

	ret = wlcore_probe_of(spi, glue, pdev_data);
	if (ret) {
		dev_err(glue->dev,