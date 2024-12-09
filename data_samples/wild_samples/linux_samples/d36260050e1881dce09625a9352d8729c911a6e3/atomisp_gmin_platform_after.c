{
	struct gmin_subdev *gs = find_gmin_subdev(subdev);

	if (!gs || gs->v1p2_on == on)
		return 0;
	gs->v1p2_on = on;

	if (gs->v1p2_reg) {
		if (on)
			return regulator_enable(gs->v1p2_reg);
		else
			return regulator_disable(gs->v1p2_reg);
	}
		if (v1p8_gpio != V1P8_GPIO_NONE) {
			pr_info("atomisp_gmin_platform: 1.8v power on GPIO %d\n",
				v1p8_gpio);
			ret = gpio_request(v1p8_gpio, "camera_v1p8_en");
			if (!ret)
				ret = gpio_direction_output(v1p8_gpio, 0);
			if (ret)
				pr_err("V1P8 GPIO initialization failed\n");
		}
	}

	if (!gs || gs->v1p8_on == on)
		return 0;
	gs->v1p8_on = on;

	if (v1p8_gpio >= 0)
		gpio_set_value(v1p8_gpio, on);

	if (gs->v1p8_reg) {
		regulator_set_voltage(gs->v1p8_reg, 1800000, 1800000);
		if (on)
			return regulator_enable(gs->v1p8_reg);
		else
			return regulator_disable(gs->v1p8_reg);
	}

	return -EINVAL;
}

static int gmin_v2p8_ctrl(struct v4l2_subdev *subdev, int on)
{
	struct gmin_subdev *gs = find_gmin_subdev(subdev);
	int ret;

	if (v2p8_gpio == V2P8_GPIO_UNSET) {
		v2p8_gpio = gmin_get_var_int(NULL, "V2P8GPIO", V2P8_GPIO_NONE);
		if (v2p8_gpio != V2P8_GPIO_NONE) {
			pr_info("atomisp_gmin_platform: 2.8v power on GPIO %d\n",
				v2p8_gpio);
			ret = gpio_request(v2p8_gpio, "camera_v2p8");
			if (!ret)
				ret = gpio_direction_output(v2p8_gpio, 0);
			if (ret)
				pr_err("V2P8 GPIO initialization failed\n");
		}
		if (v2p8_gpio != V2P8_GPIO_NONE) {
			pr_info("atomisp_gmin_platform: 2.8v power on GPIO %d\n",
				v2p8_gpio);
			ret = gpio_request(v2p8_gpio, "camera_v2p8");
			if (!ret)
				ret = gpio_direction_output(v2p8_gpio, 0);
			if (ret)
				pr_err("V2P8 GPIO initialization failed\n");
		}
	}

	if (!gs || gs->v2p8_on == on)
		return 0;
	gs->v2p8_on = on;

	if (v2p8_gpio >= 0)
		gpio_set_value(v2p8_gpio, on);

	if (gs->v2p8_reg) {
		regulator_set_voltage(gs->v2p8_reg, 2900000, 2900000);
		if (on)
			return regulator_enable(gs->v2p8_reg);
		else
			return regulator_disable(gs->v2p8_reg);
	}

	return -EINVAL;
}

static int gmin_flisclk_ctrl(struct v4l2_subdev *subdev, int on)
{
	int ret = 0;
	struct gmin_subdev *gs = find_gmin_subdev(subdev);
	struct i2c_client *client = v4l2_get_subdevdata(subdev);

	if (gs->clock_on == !!on)
		return 0;

	if (on) {
		ret = clk_set_rate(gs->pmc_clk, gs->clock_src);

		if (ret)
			dev_err(&client->dev, "unable to set PMC rate %d\n",
				gs->clock_src);

		ret = clk_prepare_enable(gs->pmc_clk);
		if (ret == 0)
			gs->clock_on = true;
	} else {
		clk_disable_unprepare(gs->pmc_clk);
		gs->clock_on = false;
	}
{
	char var8[CFG_VAR_NAME_MAX];
	efi_char16_t var16[CFG_VAR_NAME_MAX];
	struct efivar_entry *ev;
	const struct dmi_system_id *id;
	int i, ret;

	if (dev && ACPI_COMPANION(dev))
		dev = &ACPI_COMPANION(dev)->dev;

	if (dev)
		ret = snprintf(var8, sizeof(var8), "%s_%s", dev_name(dev), var);
	else
		ret = snprintf(var8, sizeof(var8), "gmin_%s", var);

	if (ret < 0 || ret >= sizeof(var8) - 1)
		return -EINVAL;

	/* First check a hard-coded list of board-specific variables.
	 * Some device firmwares lack the ability to set EFI variables at
	 * runtime.
	 */
	id = dmi_first_match(gmin_vars);
	if (id)
		return gmin_get_hardcoded_var(id->driver_data, var8, out, out_len);

	/* Our variable names are ASCII by construction, but EFI names
	 * are wide chars.  Convert and zero-pad.
	 */
	memset(var16, 0, sizeof(var16));
	for (i = 0; i < sizeof(var8) && var8[i]; i++)
		var16[i] = var8[i];

#ifdef CONFIG_64BIT
	/* To avoid owerflows when calling the efivar API */
	if (*out_len > ULONG_MAX)
		return -EINVAL;
#endif

	/* Not sure this API usage is kosher; efivar_entry_get()'s
	 * implementation simply uses VariableName and VendorGuid from
	 * the struct and ignores the rest, but it seems like there
	 * ought to be an "official" efivar_entry registered
	 * somewhere?
	 */
	ev = kzalloc(sizeof(*ev), GFP_KERNEL);
	if (!ev)
		return -ENOMEM;
	memcpy(&ev->var.VariableName, var16, sizeof(var16));
	ev->var.VendorGuid = GMIN_CFG_VAR_EFI_GUID;
	ev->var.DataSize = *out_len;

	ret = efivar_entry_get(ev, &ev->var.Attributes,
			       &ev->var.DataSize, ev->var.Data);
	if (ret == 0) {
		memcpy(out, ev->var.Data, ev->var.DataSize);
		*out_len = ev->var.DataSize;
	} else if (dev) {
		dev_warn(dev, "Failed to find gmin variable %s\n", var8);
	}

	kfree(ev);

	return ret;
}