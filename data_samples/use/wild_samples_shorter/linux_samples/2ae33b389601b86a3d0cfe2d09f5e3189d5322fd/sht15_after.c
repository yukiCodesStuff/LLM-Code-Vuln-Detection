		if (voltage)
			data->supply_uv = voltage;

		ret = regulator_enable(data->reg);
		if (ret != 0) {
			dev_err(&pdev->dev,
				"failed to enable regulator: %d\n", ret);
			return ret;
		}

		/*
		 * Setup a notifier block to update this if another device
		 * causes the voltage to change
		 */