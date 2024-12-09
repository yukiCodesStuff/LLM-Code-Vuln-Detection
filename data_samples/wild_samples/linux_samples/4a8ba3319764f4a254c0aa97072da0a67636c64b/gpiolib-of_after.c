{
	struct gg_data *gg_data = data;
	int ret;

	if ((gc->of_node != gg_data->gpiospec.np) ||
	    (gc->of_gpio_n_cells != gg_data->gpiospec.args_count) ||
	    (!gc->of_xlate))
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