	while (!num_dividers || i < num_dividers) {
		if (div_table[i] == -1)
			break;
		if (div_table[i])
			valid_div++;
		i++;
	}

	num_dividers = i;

	tmp = kcalloc(valid_div + 1, sizeof(*tmp), GFP_KERNEL);
	if (!tmp) {
		*table = ERR_PTR(-ENOMEM);
		return -ENOMEM;
	}
{
	struct clk_omap_divider *div;
	struct clk_omap_reg *reg;
	int ret;

	if (!setup)
		return NULL;

	div = kzalloc(sizeof(*div), GFP_KERNEL);
	if (!div)
		return ERR_PTR(-ENOMEM);

	reg = (struct clk_omap_reg *)&div->reg;
	reg->index = setup->module;
	reg->offset = setup->reg;

	if (setup->flags & CLKF_INDEX_STARTS_AT_ONE)
		div->flags |= CLK_DIVIDER_ONE_BASED;

	if (setup->flags & CLKF_INDEX_POWER_OF_TWO)
		div->flags |= CLK_DIVIDER_POWER_OF_TWO;

	div->table = _get_div_table_from_setup(setup, &div->width);
	if (IS_ERR(div->table)) {
		ret = PTR_ERR(div->table);
		kfree(div);
		return ERR_PTR(ret);
	}
{
	struct clk_omap_divider *div;
	struct clk_omap_reg *reg;
	int ret;

	if (!setup)
		return NULL;

	div = kzalloc(sizeof(*div), GFP_KERNEL);
	if (!div)
		return ERR_PTR(-ENOMEM);

	reg = (struct clk_omap_reg *)&div->reg;
	reg->index = setup->module;
	reg->offset = setup->reg;

	if (setup->flags & CLKF_INDEX_STARTS_AT_ONE)
		div->flags |= CLK_DIVIDER_ONE_BASED;

	if (setup->flags & CLKF_INDEX_POWER_OF_TWO)
		div->flags |= CLK_DIVIDER_POWER_OF_TWO;

	div->table = _get_div_table_from_setup(setup, &div->width);
	if (IS_ERR(div->table)) {
		ret = PTR_ERR(div->table);
		kfree(div);
		return ERR_PTR(ret);
	}