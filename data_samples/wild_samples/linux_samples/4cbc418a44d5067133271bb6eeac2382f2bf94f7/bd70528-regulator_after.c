static const struct regulator_ops bd70528_ldo_ops = {
	.enable = regulator_enable_regmap,
	.disable = regulator_disable_regmap,
	.is_enabled = regulator_is_enabled_regmap,
	.list_voltage = regulator_list_voltage_linear_range,
	.set_voltage_sel = regulator_set_voltage_sel_regmap,
	.get_voltage_sel = regulator_get_voltage_sel_regmap,
	.set_voltage_time_sel = regulator_set_voltage_time_sel,
};

static const struct regulator_ops bd70528_led_ops = {
	.enable = regulator_enable_regmap,
	.disable = regulator_disable_regmap,
	.is_enabled = regulator_is_enabled_regmap,
	.list_voltage = regulator_list_voltage_table,
	.set_voltage_sel = bd70528_led_set_voltage_sel,
	.get_voltage_sel = regulator_get_voltage_sel_regmap,
};

static const struct regulator_desc bd70528_desc[] = {
	{
		.name = "buck1",
		.of_match = of_match_ptr("BUCK1"),
		.regulators_node = of_match_ptr("regulators"),
		.id = BD70528_BUCK1,
		.ops = &bd70528_buck_ops,
		.type = REGULATOR_VOLTAGE,
		.linear_ranges = bd70528_buck1_volts,
		.n_linear_ranges = ARRAY_SIZE(bd70528_buck1_volts),
		.n_voltages = BD70528_BUCK_VOLTS,
		.enable_reg = BD70528_REG_BUCK1_EN,
		.enable_mask = BD70528_MASK_RUN_EN,
		.vsel_reg = BD70528_REG_BUCK1_VOLT,
		.vsel_mask = BD70528_MASK_BUCK_VOLT,
		.owner = THIS_MODULE,
	},
	{
		.name = "buck2",
		.of_match = of_match_ptr("BUCK2"),
		.regulators_node = of_match_ptr("regulators"),
		.id = BD70528_BUCK2,
		.ops = &bd70528_buck_ops,
		.type = REGULATOR_VOLTAGE,
		.linear_ranges = bd70528_buck2_volts,
		.n_linear_ranges = ARRAY_SIZE(bd70528_buck2_volts),
		.n_voltages = BD70528_BUCK_VOLTS,
		.enable_reg = BD70528_REG_BUCK2_EN,
		.enable_mask = BD70528_MASK_RUN_EN,
		.vsel_reg = BD70528_REG_BUCK2_VOLT,
		.vsel_mask = BD70528_MASK_BUCK_VOLT,
		.owner = THIS_MODULE,
	},
	{
		.name = "buck3",
		.of_match = of_match_ptr("BUCK3"),
		.regulators_node = of_match_ptr("regulators"),
		.id = BD70528_BUCK3,
		.ops = &bd70528_buck_ops,
		.type = REGULATOR_VOLTAGE,
		.linear_ranges = bd70528_buck3_volts,
		.n_linear_ranges = ARRAY_SIZE(bd70528_buck3_volts),
		.n_voltages = BD70528_BUCK_VOLTS,
		.enable_reg = BD70528_REG_BUCK3_EN,
		.enable_mask = BD70528_MASK_RUN_EN,
		.vsel_reg = BD70528_REG_BUCK3_VOLT,
		.vsel_mask = BD70528_MASK_BUCK_VOLT,
		.owner = THIS_MODULE,
	},
	{
		.name = "ldo1",
		.of_match = of_match_ptr("LDO1"),
		.regulators_node = of_match_ptr("regulators"),
		.id = BD70528_LDO1,
		.ops = &bd70528_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.linear_ranges = bd70528_ldo_volts,
		.n_linear_ranges = ARRAY_SIZE(bd70528_ldo_volts),
		.n_voltages = BD70528_LDO_VOLTS,
		.enable_reg = BD70528_REG_LDO1_EN,
		.enable_mask = BD70528_MASK_RUN_EN,
		.vsel_reg = BD70528_REG_LDO1_VOLT,
		.vsel_mask = BD70528_MASK_LDO_VOLT,
		.owner = THIS_MODULE,
	},
	{
		.name = "ldo2",
		.of_match = of_match_ptr("LDO2"),
		.regulators_node = of_match_ptr("regulators"),
		.id = BD70528_LDO2,
		.ops = &bd70528_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.linear_ranges = bd70528_ldo_volts,
		.n_linear_ranges = ARRAY_SIZE(bd70528_ldo_volts),
		.n_voltages = BD70528_LDO_VOLTS,
		.enable_reg = BD70528_REG_LDO2_EN,
		.enable_mask = BD70528_MASK_RUN_EN,
		.vsel_reg = BD70528_REG_LDO2_VOLT,
		.vsel_mask = BD70528_MASK_LDO_VOLT,
		.owner = THIS_MODULE,
	},
	{
		.name = "ldo3",
		.of_match = of_match_ptr("LDO3"),
		.regulators_node = of_match_ptr("regulators"),
		.id = BD70528_LDO3,
		.ops = &bd70528_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.linear_ranges = bd70528_ldo_volts,
		.n_linear_ranges = ARRAY_SIZE(bd70528_ldo_volts),
		.n_voltages = BD70528_LDO_VOLTS,
		.enable_reg = BD70528_REG_LDO3_EN,
		.enable_mask = BD70528_MASK_RUN_EN,
		.vsel_reg = BD70528_REG_LDO3_VOLT,
		.vsel_mask = BD70528_MASK_LDO_VOLT,
		.owner = THIS_MODULE,
	},
	{
		.name = "ldo_led1",
		.of_match = of_match_ptr("LDO_LED1"),
		.regulators_node = of_match_ptr("regulators"),
		.id = BD70528_LED1,
		.ops = &bd70528_led_ops,
		.type = REGULATOR_VOLTAGE,
		.volt_table = &led_volts[0],
		.n_voltages = ARRAY_SIZE(led_volts),
		.enable_reg = BD70528_REG_LED_EN,
		.enable_mask = BD70528_MASK_LED1_EN,
		.vsel_reg = BD70528_REG_LED_VOLT,
		.vsel_mask = BD70528_MASK_LED1_VOLT,
		.owner = THIS_MODULE,
	},
	{
		.name = "ldo_led2",
		.of_match = of_match_ptr("LDO_LED2"),
		.regulators_node = of_match_ptr("regulators"),
		.id = BD70528_LED2,
		.ops = &bd70528_led_ops,
		.type = REGULATOR_VOLTAGE,
		.volt_table = &led_volts[0],
		.n_voltages = ARRAY_SIZE(led_volts),
		.enable_reg = BD70528_REG_LED_EN,
		.enable_mask = BD70528_MASK_LED2_EN,
		.vsel_reg = BD70528_REG_LED_VOLT,
		.vsel_mask = BD70528_MASK_LED2_VOLT,
		.owner = THIS_MODULE,
	},

};

static int bd70528_probe(struct platform_device *pdev)
{
	struct rohm_regmap_dev *bd70528;
	int i;
	struct regulator_config config = {
		.dev = pdev->dev.parent,
	};

	bd70528 = dev_get_drvdata(pdev->dev.parent);
	if (!bd70528) {
		dev_err(&pdev->dev, "No MFD driver data\n");
		return -EINVAL;
	}

	config.regmap = bd70528->regmap;

	for (i = 0; i < ARRAY_SIZE(bd70528_desc); i++) {
		struct regulator_dev *rdev;

		rdev = devm_regulator_register(&pdev->dev, &bd70528_desc[i],
					       &config);
		if (IS_ERR(rdev)) {
			dev_err(&pdev->dev,
				"failed to register %s regulator\n",
				bd70528_desc[i].name);
			return PTR_ERR(rdev);
		}
	}
	return 0;
}

static struct platform_driver bd70528_regulator = {
	.driver = {
		.name = "bd70528-pmic"
	},
	.probe = bd70528_probe,
};

module_platform_driver(bd70528_regulator);

MODULE_AUTHOR("Matti Vaittinen <matti.vaittinen@fi.rohmeurope.com>");
MODULE_DESCRIPTION("BD70528 voltage regulator driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:bd70528-pmic");