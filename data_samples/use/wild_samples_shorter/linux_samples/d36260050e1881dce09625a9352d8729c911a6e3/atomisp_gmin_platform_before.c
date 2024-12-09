{
	struct gmin_subdev *gs = find_gmin_subdev(subdev);

	if (gs && gs->v1p2_on == on)
		return 0;
	gs->v1p2_on = on;

	if (gs->v1p2_reg) {
		}
	}

	if (gs && gs->v1p8_on == on)
		return 0;
	gs->v1p8_on = on;

	if (v1p8_gpio >= 0)
		}
	}

	if (gs && gs->v2p8_on == on)
		return 0;
	gs->v2p8_on = on;

	if (v2p8_gpio >= 0)
	for (i = 0; i < sizeof(var8) && var8[i]; i++)
		var16[i] = var8[i];

	/* To avoid owerflows when calling the efivar API */
	if (*out_len > ULONG_MAX)
		return -EINVAL;

	/* Not sure this API usage is kosher; efivar_entry_get()'s
	 * implementation simply uses VariableName and VendorGuid from
	 * the struct and ignores the rest, but it seems like there