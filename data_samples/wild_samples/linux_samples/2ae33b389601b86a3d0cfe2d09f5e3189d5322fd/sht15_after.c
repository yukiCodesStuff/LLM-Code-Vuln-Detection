	if (!IS_ERR(data->reg)) {
		int voltage;

		voltage = regulator_get_voltage(data->reg);
		if (voltage)
			data->supply_uv = voltage;

		ret = regulator_enable(data->reg);
		if (ret != 0) {
			dev_err(&pdev->dev,
				"failed to enable regulator: %d\n", ret);
			return ret;
		}