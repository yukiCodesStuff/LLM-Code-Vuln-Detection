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
	if (PTR_ERR(glue->reg) == -EPROBE_DEFER)
		return -EPROBE_DEFER;
	if (IS_ERR(glue->reg)) {
		dev_err(glue->dev, "can't get regulator\n");
		return PTR_ERR(glue->reg);
	}