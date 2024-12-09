{
	const struct rockchip_cpuclk_reg_data *reg_data = cpuclk->reg_data;
	unsigned long alt_prate, alt_div;
	unsigned long flags;

	alt_prate = clk_get_rate(cpuclk->alt_parent);

	spin_lock_irqsave(cpuclk->lock, flags);

	/*
	 * If the old parent clock speed is less than the clock speed
	 * of the alternate parent, then it should be ensured that at no point
	 * the armclk speed is more than the old_rate until the dividers are
	 * set.
	 */
	if (alt_prate > ndata->old_rate) {
		/* calculate dividers */
		alt_div =  DIV_ROUND_UP(alt_prate, ndata->old_rate) - 1;
		if (alt_div > reg_data->div_core_mask) {
			pr_warn("%s: limiting alt-divider %lu to %d\n",
				__func__, alt_div, reg_data->div_core_mask);
			alt_div = reg_data->div_core_mask;
		}

		/*
		 * Change parents and add dividers in a single transaction.
		 *
		 * NOTE: we do this in a single transaction so we're never
		 * dividing the primary parent by the extra dividers that were
		 * needed for the alt.
		 */
		pr_debug("%s: setting div %lu as alt-rate %lu > old-rate %lu\n",
			 __func__, alt_div, alt_prate, ndata->old_rate);

		writel(HIWORD_UPDATE(alt_div, reg_data->div_core_mask,
					      reg_data->div_core_shift) |
		       HIWORD_UPDATE(1, 1, reg_data->mux_core_shift),
		       cpuclk->reg_base + reg_data->core_reg);
	} else {
		/* select alternate parent */
		writel(HIWORD_UPDATE(1, 1, reg_data->mux_core_shift),
			cpuclk->reg_base + reg_data->core_reg);
	}

	spin_unlock_irqrestore(cpuclk->lock, flags);
	return 0;
}
	} else {
		/* select alternate parent */
		writel(HIWORD_UPDATE(1, 1, reg_data->mux_core_shift),
			cpuclk->reg_base + reg_data->core_reg);
	}

	spin_unlock_irqrestore(cpuclk->lock, flags);
	return 0;
}

static int rockchip_cpuclk_post_rate_change(struct rockchip_cpuclk *cpuclk,
					    struct clk_notifier_data *ndata)
{
	const struct rockchip_cpuclk_reg_data *reg_data = cpuclk->reg_data;
	const struct rockchip_cpuclk_rate_table *rate;
	unsigned long flags;

	rate = rockchip_get_cpuclk_settings(cpuclk, ndata->new_rate);
	if (!rate) {
		pr_err("%s: Invalid rate : %lu for cpuclk\n",
		       __func__, ndata->new_rate);
		return -EINVAL;
	}

	spin_lock_irqsave(cpuclk->lock, flags);

	if (ndata->old_rate < ndata->new_rate)
		rockchip_cpuclk_set_dividers(cpuclk, rate);

	/*
	 * post-rate change event, re-mux to primary parent and remove dividers.
	 *
	 * NOTE: we do this in a single transaction so we're never dividing the
	 * primary parent by the extra dividers that were needed for the alt.
	 */

	writel(HIWORD_UPDATE(0, reg_data->div_core_mask,
				reg_data->div_core_shift) |
	       HIWORD_UPDATE(0, 1, reg_data->mux_core_shift),
	       cpuclk->reg_base + reg_data->core_reg);

	if (ndata->old_rate > ndata->new_rate)
		rockchip_cpuclk_set_dividers(cpuclk, rate);

	spin_unlock_irqrestore(cpuclk->lock, flags);
	return 0;
}

/*
 * This clock notifier is called when the frequency of the parent clock
 * of cpuclk is to be changed. This notifier handles the setting up all
 * the divider clocks, remux to temporary parent and handling the safe
 * frequency levels when using temporary parent.
 */
static int rockchip_cpuclk_notifier_cb(struct notifier_block *nb,
					unsigned long event, void *data)
{
	struct clk_notifier_data *ndata = data;
	struct rockchip_cpuclk *cpuclk = to_rockchip_cpuclk_nb(nb);
	int ret = 0;

	pr_debug("%s: event %lu, old_rate %lu, new_rate: %lu\n",
		 __func__, event, ndata->old_rate, ndata->new_rate);
	if (event == PRE_RATE_CHANGE)
		ret = rockchip_cpuclk_pre_rate_change(cpuclk, ndata);
	else if (event == POST_RATE_CHANGE)
		ret = rockchip_cpuclk_post_rate_change(cpuclk, ndata);

	return notifier_from_errno(ret);
}

struct clk *rockchip_clk_register_cpuclk(const char *name,
			const char **parent_names, u8 num_parents,
			const struct rockchip_cpuclk_reg_data *reg_data,
			const struct rockchip_cpuclk_rate_table *rates,
			int nrates, void __iomem *reg_base, spinlock_t *lock)
{
	struct rockchip_cpuclk *cpuclk;
	struct clk_init_data init;
	struct clk *clk, *cclk;
	int ret;

	if (num_parents != 2) {
		pr_err("%s: needs two parent clocks\n", __func__);
		return ERR_PTR(-EINVAL);
	}

	cpuclk = kzalloc(sizeof(*cpuclk), GFP_KERNEL);
	if (!cpuclk)
		return ERR_PTR(-ENOMEM);

	init.name = name;
	init.parent_names = &parent_names[0];
	init.num_parents = 1;
	init.ops = &rockchip_cpuclk_ops;

	/* only allow rate changes when we have a rate table */
	init.flags = (nrates > 0) ? CLK_SET_RATE_PARENT : 0;

	/* disallow automatic parent changes by ccf */
	init.flags |= CLK_SET_RATE_NO_REPARENT;

	init.flags |= CLK_GET_RATE_NOCACHE;

	cpuclk->reg_base = reg_base;
	cpuclk->lock = lock;
	cpuclk->reg_data = reg_data;
	cpuclk->clk_nb.notifier_call = rockchip_cpuclk_notifier_cb;
	cpuclk->hw.init = &init;

	cpuclk->alt_parent = __clk_lookup(parent_names[1]);
	if (!cpuclk->alt_parent) {
		pr_err("%s: could not lookup alternate parent\n",
		       __func__);
		ret = -EINVAL;
		goto free_cpuclk;
	}

	ret = clk_prepare_enable(cpuclk->alt_parent);
	if (ret) {
		pr_err("%s: could not enable alternate parent\n",
		       __func__);
		goto free_cpuclk;
	}

	clk = __clk_lookup(parent_names[0]);
	if (!clk) {
		pr_err("%s: could not lookup parent clock %s\n",
		       __func__, parent_names[0]);
		ret = -EINVAL;
		goto free_cpuclk;
	}

	ret = clk_notifier_register(clk, &cpuclk->clk_nb);
	if (ret) {
		pr_err("%s: failed to register clock notifier for %s\n",
				__func__, name);
		goto free_cpuclk;
	}

	if (nrates > 0) {
		cpuclk->rate_count = nrates;
		cpuclk->rate_table = kmemdup(rates,
					     sizeof(*rates) * nrates,
					     GFP_KERNEL);
		if (!cpuclk->rate_table) {
			pr_err("%s: could not allocate memory for cpuclk rates\n",
			       __func__);
			ret = -ENOMEM;
			goto unregister_notifier;
		}
{
	const struct rockchip_cpuclk_reg_data *reg_data = cpuclk->reg_data;
	const struct rockchip_cpuclk_rate_table *rate;
	unsigned long flags;

	rate = rockchip_get_cpuclk_settings(cpuclk, ndata->new_rate);
	if (!rate) {
		pr_err("%s: Invalid rate : %lu for cpuclk\n",
		       __func__, ndata->new_rate);
		return -EINVAL;
	}
	if (!rate) {
		pr_err("%s: Invalid rate : %lu for cpuclk\n",
		       __func__, ndata->new_rate);
		return -EINVAL;
	}

	spin_lock_irqsave(cpuclk->lock, flags);

	if (ndata->old_rate < ndata->new_rate)
		rockchip_cpuclk_set_dividers(cpuclk, rate);

	/*
	 * post-rate change event, re-mux to primary parent and remove dividers.
	 *
	 * NOTE: we do this in a single transaction so we're never dividing the
	 * primary parent by the extra dividers that were needed for the alt.
	 */

	writel(HIWORD_UPDATE(0, reg_data->div_core_mask,
				reg_data->div_core_shift) |
	       HIWORD_UPDATE(0, 1, reg_data->mux_core_shift),
	       cpuclk->reg_base + reg_data->core_reg);

	if (ndata->old_rate > ndata->new_rate)
		rockchip_cpuclk_set_dividers(cpuclk, rate);

	spin_unlock_irqrestore(cpuclk->lock, flags);
	return 0;
}

/*
 * This clock notifier is called when the frequency of the parent clock
 * of cpuclk is to be changed. This notifier handles the setting up all
 * the divider clocks, remux to temporary parent and handling the safe
 * frequency levels when using temporary parent.
 */
static int rockchip_cpuclk_notifier_cb(struct notifier_block *nb,
					unsigned long event, void *data)
{
	struct clk_notifier_data *ndata = data;
	struct rockchip_cpuclk *cpuclk = to_rockchip_cpuclk_nb(nb);
	int ret = 0;

	pr_debug("%s: event %lu, old_rate %lu, new_rate: %lu\n",
		 __func__, event, ndata->old_rate, ndata->new_rate);
	if (event == PRE_RATE_CHANGE)
		ret = rockchip_cpuclk_pre_rate_change(cpuclk, ndata);
	else if (event == POST_RATE_CHANGE)
		ret = rockchip_cpuclk_post_rate_change(cpuclk, ndata);

	return notifier_from_errno(ret);
}

struct clk *rockchip_clk_register_cpuclk(const char *name,
			const char **parent_names, u8 num_parents,
			const struct rockchip_cpuclk_reg_data *reg_data,
			const struct rockchip_cpuclk_rate_table *rates,
			int nrates, void __iomem *reg_base, spinlock_t *lock)
{
	struct rockchip_cpuclk *cpuclk;
	struct clk_init_data init;
	struct clk *clk, *cclk;
	int ret;

	if (num_parents != 2) {
		pr_err("%s: needs two parent clocks\n", __func__);
		return ERR_PTR(-EINVAL);
	}

	cpuclk = kzalloc(sizeof(*cpuclk), GFP_KERNEL);
	if (!cpuclk)
		return ERR_PTR(-ENOMEM);

	init.name = name;
	init.parent_names = &parent_names[0];
	init.num_parents = 1;
	init.ops = &rockchip_cpuclk_ops;

	/* only allow rate changes when we have a rate table */
	init.flags = (nrates > 0) ? CLK_SET_RATE_PARENT : 0;

	/* disallow automatic parent changes by ccf */
	init.flags |= CLK_SET_RATE_NO_REPARENT;

	init.flags |= CLK_GET_RATE_NOCACHE;

	cpuclk->reg_base = reg_base;
	cpuclk->lock = lock;
	cpuclk->reg_data = reg_data;
	cpuclk->clk_nb.notifier_call = rockchip_cpuclk_notifier_cb;
	cpuclk->hw.init = &init;

	cpuclk->alt_parent = __clk_lookup(parent_names[1]);
	if (!cpuclk->alt_parent) {
		pr_err("%s: could not lookup alternate parent\n",
		       __func__);
		ret = -EINVAL;
		goto free_cpuclk;
	}

	ret = clk_prepare_enable(cpuclk->alt_parent);
	if (ret) {
		pr_err("%s: could not enable alternate parent\n",
		       __func__);
		goto free_cpuclk;
	}

	clk = __clk_lookup(parent_names[0]);
	if (!clk) {
		pr_err("%s: could not lookup parent clock %s\n",
		       __func__, parent_names[0]);
		ret = -EINVAL;
		goto free_cpuclk;
	}

	ret = clk_notifier_register(clk, &cpuclk->clk_nb);
	if (ret) {
		pr_err("%s: failed to register clock notifier for %s\n",
				__func__, name);
		goto free_cpuclk;
	}

	if (nrates > 0) {
		cpuclk->rate_count = nrates;
		cpuclk->rate_table = kmemdup(rates,
					     sizeof(*rates) * nrates,
					     GFP_KERNEL);
		if (!cpuclk->rate_table) {
			pr_err("%s: could not allocate memory for cpuclk rates\n",
			       __func__);
			ret = -ENOMEM;
			goto unregister_notifier;
		}
	if (!rate) {
		pr_err("%s: Invalid rate : %lu for cpuclk\n",
		       __func__, ndata->new_rate);
		return -EINVAL;
	}

	spin_lock_irqsave(cpuclk->lock, flags);

	if (ndata->old_rate < ndata->new_rate)
		rockchip_cpuclk_set_dividers(cpuclk, rate);

	/*
	 * post-rate change event, re-mux to primary parent and remove dividers.
	 *
	 * NOTE: we do this in a single transaction so we're never dividing the
	 * primary parent by the extra dividers that were needed for the alt.
	 */

	writel(HIWORD_UPDATE(0, reg_data->div_core_mask,
				reg_data->div_core_shift) |
	       HIWORD_UPDATE(0, 1, reg_data->mux_core_shift),
	       cpuclk->reg_base + reg_data->core_reg);

	if (ndata->old_rate > ndata->new_rate)
		rockchip_cpuclk_set_dividers(cpuclk, rate);

	spin_unlock_irqrestore(cpuclk->lock, flags);
	return 0;
}

/*
 * This clock notifier is called when the frequency of the parent clock
 * of cpuclk is to be changed. This notifier handles the setting up all
 * the divider clocks, remux to temporary parent and handling the safe
 * frequency levels when using temporary parent.
 */
static int rockchip_cpuclk_notifier_cb(struct notifier_block *nb,
					unsigned long event, void *data)
{
	struct clk_notifier_data *ndata = data;
	struct rockchip_cpuclk *cpuclk = to_rockchip_cpuclk_nb(nb);
	int ret = 0;

	pr_debug("%s: event %lu, old_rate %lu, new_rate: %lu\n",
		 __func__, event, ndata->old_rate, ndata->new_rate);
	if (event == PRE_RATE_CHANGE)
		ret = rockchip_cpuclk_pre_rate_change(cpuclk, ndata);
	else if (event == POST_RATE_CHANGE)
		ret = rockchip_cpuclk_post_rate_change(cpuclk, ndata);

	return notifier_from_errno(ret);
}

struct clk *rockchip_clk_register_cpuclk(const char *name,
			const char **parent_names, u8 num_parents,
			const struct rockchip_cpuclk_reg_data *reg_data,
			const struct rockchip_cpuclk_rate_table *rates,
			int nrates, void __iomem *reg_base, spinlock_t *lock)
{
	struct rockchip_cpuclk *cpuclk;
	struct clk_init_data init;
	struct clk *clk, *cclk;
	int ret;

	if (num_parents != 2) {
		pr_err("%s: needs two parent clocks\n", __func__);
		return ERR_PTR(-EINVAL);
	}