	/* TODO: remove, see function definition */
	gpmc_convert_ps_to_ns(gpmc_t);

	return 0;
}

#ifdef CONFIG_OF
	if (IS_ERR_VALUE(gpmc_setup_irq()))
		dev_warn(gpmc_dev, "gpmc_setup_irq failed\n");

	/* Now the GPMC is initialised, unreserve the chip-selects */
	gpmc_cs_map = 0;

	rc = gpmc_probe_dt(pdev);
	if (rc < 0) {
		clk_disable_unprepare(gpmc_l3_clk);
		clk_put(gpmc_l3_clk);