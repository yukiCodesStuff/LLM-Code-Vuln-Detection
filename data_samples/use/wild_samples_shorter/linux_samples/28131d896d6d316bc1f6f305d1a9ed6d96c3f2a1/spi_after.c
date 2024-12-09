	spi->bits_per_word = 32;

	glue->reg = devm_regulator_get(&spi->dev, "vwlan");
	if (IS_ERR(glue->reg))
		return dev_err_probe(glue->dev, PTR_ERR(glue->reg),
				     "can't get regulator\n");

	ret = wlcore_probe_of(spi, glue, pdev_data);
	if (ret) {
		dev_err(glue->dev,