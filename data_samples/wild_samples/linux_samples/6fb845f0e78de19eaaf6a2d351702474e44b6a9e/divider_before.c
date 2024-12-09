	while (!num_dividers || i < num_dividers) {
		if (div_table[i] == -1)
			break;
		if (div_table[i])
			valid_div++;
		i++;
	}

	num_dividers = i;

	tmp = kcalloc(valid_div + 1, sizeof(*tmp), GFP_KERNEL);
	if (!tmp)
		return -ENOMEM;

	valid_div = 0;
	*width = 0;

	for (i = 0; i < num_dividers; i++)
		if (div_table[i] > 0) {
			tmp[valid_div].div = div_table[i];
			tmp[valid_div].val = i;
			valid_div++;
			*width = i;
		}
{
	struct clk_omap_divider *div;
	struct clk_omap_reg *reg;

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

	div->shift = setup->bit_shift;
	div->latch = -EINVAL;

	return &div->hw;
}

struct clk *ti_clk_register_divider(struct ti_clk *setup)
{
	struct ti_clk_divider *div = setup->data;
	struct clk_omap_reg reg = {
		.index = div->module,
		.offset = div->reg,
	};
	u8 width;
	u32 flags = 0;
	u8 div_flags = 0;
	const struct clk_div_table *table;
	struct clk *clk;

	if (div->flags & CLKF_INDEX_STARTS_AT_ONE)
		div_flags |= CLK_DIVIDER_ONE_BASED;

	if (div->flags & CLKF_INDEX_POWER_OF_TWO)
		div_flags |= CLK_DIVIDER_POWER_OF_TWO;

	if (div->flags & CLKF_SET_RATE_PARENT)
		flags |= CLK_SET_RATE_PARENT;

	table = _get_div_table_from_setup(div, &width);
	if (IS_ERR(table))
		return (struct clk *)table;

	clk = _register_divider(NULL, setup->name, div->parent,
				flags, &reg, div->bit_shift,
				width, -EINVAL, div_flags, table);

	if (IS_ERR(clk))
		kfree(table);

	return clk;
}
{
	struct clk_omap_divider *div;
	struct clk_omap_reg *reg;

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

	div->shift = setup->bit_shift;
	div->latch = -EINVAL;

	return &div->hw;
}

struct clk *ti_clk_register_divider(struct ti_clk *setup)
{
	struct ti_clk_divider *div = setup->data;
	struct clk_omap_reg reg = {
		.index = div->module,
		.offset = div->reg,
	};
	u8 width;
	u32 flags = 0;
	u8 div_flags = 0;
	const struct clk_div_table *table;
	struct clk *clk;

	if (div->flags & CLKF_INDEX_STARTS_AT_ONE)
		div_flags |= CLK_DIVIDER_ONE_BASED;

	if (div->flags & CLKF_INDEX_POWER_OF_TWO)
		div_flags |= CLK_DIVIDER_POWER_OF_TWO;

	if (div->flags & CLKF_SET_RATE_PARENT)
		flags |= CLK_SET_RATE_PARENT;

	table = _get_div_table_from_setup(div, &width);
	if (IS_ERR(table))
		return (struct clk *)table;

	clk = _register_divider(NULL, setup->name, div->parent,
				flags, &reg, div->bit_shift,
				width, -EINVAL, div_flags, table);

	if (IS_ERR(clk))
		kfree(table);

	return clk;
}