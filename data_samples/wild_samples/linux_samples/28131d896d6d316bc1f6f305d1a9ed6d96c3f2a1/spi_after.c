	if (!glue) {
		dev_err(&spi->dev, "can't allocate glue\n");
		return -ENOMEM;
	}

	glue->dev = &spi->dev;

	spi_set_drvdata(spi, glue);

	/* This is the only SPI value that we need to set here, the rest
	 * comes from the board-peripherals file */
	spi->bits_per_word = 32;

	glue->reg = devm_regulator_get(&spi->dev, "vwlan");
	if (IS_ERR(glue->reg))
		return dev_err_probe(glue->dev, PTR_ERR(glue->reg),
				     "can't get regulator\n");

	ret = wlcore_probe_of(spi, glue, pdev_data);
	if (ret) {
		dev_err(glue->dev,
			"can't get device tree parameters (%d)\n", ret);
		return ret;
	}