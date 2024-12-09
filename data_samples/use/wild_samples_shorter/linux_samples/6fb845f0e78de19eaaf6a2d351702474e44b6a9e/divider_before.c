	num_dividers = i;

	tmp = kcalloc(valid_div + 1, sizeof(*tmp), GFP_KERNEL);
	if (!tmp)
		return -ENOMEM;

	valid_div = 0;
	*width = 0;

{
	struct clk_omap_divider *div;
	struct clk_omap_reg *reg;

	if (!setup)
		return NULL;

		div->flags |= CLK_DIVIDER_POWER_OF_TWO;

	div->table = _get_div_table_from_setup(setup, &div->width);

	div->shift = setup->bit_shift;
	div->latch = -EINVAL;
