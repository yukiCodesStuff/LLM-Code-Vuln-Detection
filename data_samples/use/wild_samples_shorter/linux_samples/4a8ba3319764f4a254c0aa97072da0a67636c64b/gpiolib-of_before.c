		return false;

	ret = gc->of_xlate(gc, &gg_data->gpiospec, gg_data->flags);
	if (ret < 0)
		return false;

	gg_data->out_gpio = gpiochip_get_desc(gc, ret);
	return true;
}