		int i;

		for (i = 0; i < rate_count; i++) {
			if (ramp <= slew_rates[i])
				cfg = AXP20X_DCDC2_LDO3_V_RAMP_LDO3_RATE(i);
			else
				break;
		}

		if (cfg == 0xff) {
			dev_err(axp20x->dev, "unsupported ramp value %d", ramp);
		 AXP22X_PWR_OUT_CTRL2, AXP22X_PWR_OUT_ELDO1_MASK),
	AXP_DESC(AXP22X, ELDO2, "eldo2", "eldoin", 700, 3300, 100,
		 AXP22X_ELDO2_V_OUT, AXP22X_ELDO2_V_OUT_MASK,
		 AXP22X_PWR_OUT_CTRL2, AXP22X_PWR_OUT_ELDO1_MASK),
	AXP_DESC(AXP22X, ELDO3, "eldo3", "eldoin", 700, 3300, 100,
		 AXP22X_ELDO3_V_OUT, AXP22X_ELDO3_V_OUT_MASK,
		 AXP22X_PWR_OUT_CTRL2, AXP22X_PWR_OUT_ELDO3_MASK),
	/* Note the datasheet only guarantees reliable operation up to