struct clk_sam9x5_slow {
	struct clk_hw hw;
	void __iomem *sckcr;
	u8 parent;
};

#define to_clk_sam9x5_slow(hw) container_of(hw, struct clk_sam9x5_slow, hw)

static struct clk *slow_clk;

static int clk_slow_osc_prepare(struct clk_hw *hw)
{
	struct clk_slow_osc *osc = to_clk_slow_osc(hw);
	void __iomem *sckcr = osc->sckcr;
	u32 tmp = readl(sckcr);

	if (tmp & AT91_SCKC_OSC32BYP)
		return 0;

	writel(tmp | AT91_SCKC_OSC32EN, sckcr);

	usleep_range(osc->startup_usec, osc->startup_usec + 1);

	return 0;
}
{
	struct clk_sam9x5_slow *slowck;
	struct clk *clk = NULL;
	struct clk_init_data init;

	if (!sckcr || !name || !parent_names || !num_parents)
		return ERR_PTR(-EINVAL);

	slowck = kzalloc(sizeof(*slowck), GFP_KERNEL);
	if (!slowck)
		return ERR_PTR(-ENOMEM);

	init.name = name;
	init.ops = &sam9x5_slow_ops;
	init.parent_names = parent_names;
	init.num_parents = num_parents;
	init.flags = 0;

	slowck->hw.init = &init;
	slowck->sckcr = sckcr;
	slowck->parent = !!(readl(sckcr) & AT91_SCKC_OSCSEL);

	clk = clk_register(NULL, &slowck->hw);
	if (IS_ERR(clk))
		kfree(slowck);
	else
		slow_clk = clk;

	return clk;
}

void __init of_at91sam9x5_clk_slow_setup(struct device_node *np,
					 void __iomem *sckcr)
{
	struct clk *clk;
	const char *parent_names[2];
	int num_parents;
	const char *name = np->name;
	int i;

	num_parents = of_count_phandle_with_args(np, "clocks", "#clock-cells");
	if (num_parents <= 0 || num_parents > 2)
		return;

	for (i = 0; i < num_parents; ++i) {
		parent_names[i] = of_clk_get_parent_name(np, i);
		if (!parent_names[i])
			return;
	}
{
	struct clk_sam9260_slow *slowck;
	struct clk *clk = NULL;
	struct clk_init_data init;

	if (!pmc || !name)
		return ERR_PTR(-EINVAL);

	if (!parent_names || !num_parents)
		return ERR_PTR(-EINVAL);

	slowck = kzalloc(sizeof(*slowck), GFP_KERNEL);
	if (!slowck)
		return ERR_PTR(-ENOMEM);

	init.name = name;
	init.ops = &sam9260_slow_ops;
	init.parent_names = parent_names;
	init.num_parents = num_parents;
	init.flags = 0;

	slowck->hw.init = &init;
	slowck->pmc = pmc;

	clk = clk_register(NULL, &slowck->hw);
	if (IS_ERR(clk))
		kfree(slowck);
	else
		slow_clk = clk;

	return clk;
}

void __init of_at91sam9260_clk_slow_setup(struct device_node *np,
					  struct at91_pmc *pmc)
{
	struct clk *clk;
	const char *parent_names[2];
	int num_parents;
	const char *name = np->name;
	int i;

	num_parents = of_count_phandle_with_args(np, "clocks", "#clock-cells");
	if (num_parents != 2)
		return;

	for (i = 0; i < num_parents; ++i) {
		parent_names[i] = of_clk_get_parent_name(np, i);
		if (!parent_names[i])
			return;
	}
	for (i = 0; i < num_parents; ++i) {
		parent_names[i] = of_clk_get_parent_name(np, i);
		if (!parent_names[i])
			return;
	}

	of_property_read_string(np, "clock-output-names", &name);

	clk = at91_clk_register_sam9260_slow(pmc, name, parent_names,
					     num_parents);
	if (IS_ERR(clk))
		return;

	of_clk_add_provider(np, of_clk_src_simple_get, clk);
}

/*
 * FIXME: All slow clk users are not properly claiming it (get + prepare +
 * enable) before using it.
 * If all users properly claiming this clock decide that they don't need it
 * anymore (or are removed), it is disabled while faulty users are still
 * requiring it, and the system hangs.
 * Prevent this clock from being disabled until all users are properly
 * requesting it.
 * Once this is done we should remove this function and the slow_clk variable.
 */
static int __init of_at91_clk_slow_retain(void)
{
	if (!slow_clk)
		return 0;

	__clk_get(slow_clk);
	clk_prepare_enable(slow_clk);

	return 0;
}
arch_initcall(of_at91_clk_slow_retain);