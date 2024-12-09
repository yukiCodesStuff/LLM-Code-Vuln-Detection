{
	memset(gpmc_t, 0, sizeof(*gpmc_t));

	gpmc_calc_common_timings(gpmc_t, dev_t);

	if (dev_t->sync_read)
		gpmc_calc_sync_read_timings(gpmc_t, dev_t);
	else
		gpmc_calc_async_read_timings(gpmc_t, dev_t);

	if (dev_t->sync_write)
		gpmc_calc_sync_write_timings(gpmc_t, dev_t);
	else
		gpmc_calc_async_write_timings(gpmc_t, dev_t);

	/* TODO: remove, see function definition */
	gpmc_convert_ps_to_ns(gpmc_t);

	/* Now the GPMC is initialised, unreserve the chip-selects */
	gpmc_cs_map = 0;

	return 0;
}

#ifdef CONFIG_OF
static struct of_device_id gpmc_dt_ids[] = {
	{ .compatible = "ti,omap2420-gpmc" },
	{ .compatible = "ti,omap2430-gpmc" },
	{ .compatible = "ti,omap3430-gpmc" },	/* omap3430 & omap3630 */
	{ .compatible = "ti,omap4430-gpmc" },	/* omap4430 & omap4460 & omap543x */
	{ .compatible = "ti,am3352-gpmc" },	/* am335x devices */
	{ }
};
MODULE_DEVICE_TABLE(of, gpmc_dt_ids);

static void __maybe_unused gpmc_read_timings_dt(struct device_node *np,
						struct gpmc_timings *gpmc_t)
{
	u32 val;

	memset(gpmc_t, 0, sizeof(*gpmc_t));

	/* minimum clock period for syncronous mode */
	if (!of_property_read_u32(np, "gpmc,sync-clk", &val))
		gpmc_t->sync_clk = val;

	/* chip select timtings */
	if (!of_property_read_u32(np, "gpmc,cs-on", &val))
		gpmc_t->cs_on = val;

	if (!of_property_read_u32(np, "gpmc,cs-rd-off", &val))
		gpmc_t->cs_rd_off = val;

	if (!of_property_read_u32(np, "gpmc,cs-wr-off", &val))
		gpmc_t->cs_wr_off = val;

	/* ADV signal timings */
	if (!of_property_read_u32(np, "gpmc,adv-on", &val))
		gpmc_t->adv_on = val;

	if (!of_property_read_u32(np, "gpmc,adv-rd-off", &val))
		gpmc_t->adv_rd_off = val;

	if (!of_property_read_u32(np, "gpmc,adv-wr-off", &val))
		gpmc_t->adv_wr_off = val;

	/* WE signal timings */
	if (!of_property_read_u32(np, "gpmc,we-on", &val))
		gpmc_t->we_on = val;

	if (!of_property_read_u32(np, "gpmc,we-off", &val))
		gpmc_t->we_off = val;

	/* OE signal timings */
	if (!of_property_read_u32(np, "gpmc,oe-on", &val))
		gpmc_t->oe_on = val;

	if (!of_property_read_u32(np, "gpmc,oe-off", &val))
		gpmc_t->oe_off = val;

	/* access and cycle timings */
	if (!of_property_read_u32(np, "gpmc,page-burst-access", &val))
		gpmc_t->page_burst_access = val;

	if (!of_property_read_u32(np, "gpmc,access", &val))
		gpmc_t->access = val;

	if (!of_property_read_u32(np, "gpmc,rd-cycle", &val))
		gpmc_t->rd_cycle = val;

	if (!of_property_read_u32(np, "gpmc,wr-cycle", &val))
		gpmc_t->wr_cycle = val;

	/* only for OMAP3430 */
	if (!of_property_read_u32(np, "gpmc,wr-access", &val))
		gpmc_t->wr_access = val;

	if (!of_property_read_u32(np, "gpmc,wr-data-mux-bus", &val))
		gpmc_t->wr_data_mux_bus = val;
}

#ifdef CONFIG_MTD_NAND

static const char * const nand_ecc_opts[] = {
	[OMAP_ECC_HAMMING_CODE_DEFAULT]		= "sw",
	[OMAP_ECC_HAMMING_CODE_HW]		= "hw",
	[OMAP_ECC_HAMMING_CODE_HW_ROMCODE]	= "hw-romcode",
	[OMAP_ECC_BCH4_CODE_HW]			= "bch4",
	[OMAP_ECC_BCH8_CODE_HW]			= "bch8",
};

static int gpmc_probe_nand_child(struct platform_device *pdev,
				 struct device_node *child)
{
	u32 val;
	const char *s;
	struct gpmc_timings gpmc_t;
	struct omap_nand_platform_data *gpmc_nand_data;

	if (of_property_read_u32(child, "reg", &val) < 0) {
		dev_err(&pdev->dev, "%s has no 'reg' property\n",
			child->full_name);
		return -ENODEV;
	}

	gpmc_nand_data = devm_kzalloc(&pdev->dev, sizeof(*gpmc_nand_data),
				      GFP_KERNEL);
	if (!gpmc_nand_data)
		return -ENOMEM;

	gpmc_nand_data->cs = val;
	gpmc_nand_data->of_node = child;

	if (!of_property_read_string(child, "ti,nand-ecc-opt", &s))
		for (val = 0; val < ARRAY_SIZE(nand_ecc_opts); val++)
			if (!strcasecmp(s, nand_ecc_opts[val])) {
				gpmc_nand_data->ecc_opt = val;
				break;
			}

	val = of_get_nand_bus_width(child);
	if (val == 16)
		gpmc_nand_data->devsize = NAND_BUSWIDTH_16;

	gpmc_read_timings_dt(child, &gpmc_t);
	gpmc_nand_init(gpmc_nand_data, &gpmc_t);

	return 0;
}
	if (IS_ERR_VALUE(rc)) {
		clk_disable_unprepare(gpmc_l3_clk);
		clk_put(gpmc_l3_clk);
		dev_err(gpmc_dev, "failed to reserve memory\n");
		return rc;
	}

	if (IS_ERR_VALUE(gpmc_setup_irq()))
		dev_warn(gpmc_dev, "gpmc_setup_irq failed\n");

	rc = gpmc_probe_dt(pdev);
	if (rc < 0) {
		clk_disable_unprepare(gpmc_l3_clk);
		clk_put(gpmc_l3_clk);
		dev_err(gpmc_dev, "failed to probe DT parameters\n");
		return rc;
	}