	num_dividers = i;

	tmp = kcalloc(valid_div + 1, sizeof(*tmp), GFP_KERNEL);
	if (!tmp) {
		*table = ERR_PTR(-ENOMEM);
		return -ENOMEM;
	}

	valid_div = 0;
	*width = 0;

{
	struct clk_omap_divider *div;
	struct clk_omap_reg *reg;
	int ret;

	if (!setup)
		return NULL;

		div->flags |= CLK_DIVIDER_POWER_OF_TWO;

	div->table = _get_div_table_from_setup(setup, &div->width);
	if (IS_ERR(div->table)) {
		ret = PTR_ERR(div->table);
		kfree(div);
		return ERR_PTR(ret);
	}


	div->shift = setup->bit_shift;
	div->latch = -EINVAL;
