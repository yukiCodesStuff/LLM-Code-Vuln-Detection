	.set_voltage_sel = regulator_set_voltage_sel_regmap,
	.get_voltage_sel = regulator_get_voltage_sel_regmap,
	.set_voltage_time_sel = regulator_set_voltage_time_sel,
	.set_ramp_delay = bd70528_set_ramp_delay,
};

static const struct regulator_ops bd70528_led_ops = {
	.enable = regulator_enable_regmap,