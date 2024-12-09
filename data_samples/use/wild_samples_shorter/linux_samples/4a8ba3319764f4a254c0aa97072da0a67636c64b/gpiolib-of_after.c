		return false;

	ret = gc->of_xlate(gc, &gg_data->gpiospec, gg_data->flags);
	if (ret < 0) {
		/* We've found the gpio chip, but the translation failed.
		 * Return true to stop looking and return the translation
		 * error via out_gpio
		 */
		gg_data->out_gpio = ERR_PTR(ret);
		return true;
	 }

	gg_data->out_gpio = gpiochip_get_desc(gc, ret);
	return true;
}