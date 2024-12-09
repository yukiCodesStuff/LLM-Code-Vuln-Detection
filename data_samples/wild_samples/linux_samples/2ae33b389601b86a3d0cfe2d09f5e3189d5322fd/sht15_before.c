	if (!IS_ERR(data->reg)) {
		int voltage;

		voltage = regulator_get_voltage(data->reg);
		if (voltage)
			data->supply_uv = voltage;

		regulator_enable(data->reg);
		/*
		 * Setup a notifier block to update this if another device
		 * causes the voltage to change
		 */
		data->nb.notifier_call = &sht15_invalidate_voltage;
		ret = regulator_register_notifier(data->reg, &data->nb);
		if (ret) {
			dev_err(&pdev->dev,
				"regulator notifier request failed\n");
			regulator_disable(data->reg);
			return ret;
		}