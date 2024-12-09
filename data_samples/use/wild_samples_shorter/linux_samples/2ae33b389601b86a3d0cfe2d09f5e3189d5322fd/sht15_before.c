		if (voltage)
			data->supply_uv = voltage;

		regulator_enable(data->reg);
		/*
		 * Setup a notifier block to update this if another device
		 * causes the voltage to change
		 */