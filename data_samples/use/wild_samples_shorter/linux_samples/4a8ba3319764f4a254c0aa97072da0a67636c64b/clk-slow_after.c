
#define to_clk_sam9x5_slow(hw) container_of(hw, struct clk_sam9x5_slow, hw)

static struct clk *slow_clk;

static int clk_slow_osc_prepare(struct clk_hw *hw)
{
	struct clk_slow_osc *osc = to_clk_slow_osc(hw);
	clk = clk_register(NULL, &slowck->hw);
	if (IS_ERR(clk))
		kfree(slowck);
	else
		slow_clk = clk;

	return clk;
}

	clk = clk_register(NULL, &slowck->hw);
	if (IS_ERR(clk))
		kfree(slowck);
	else
		slow_clk = clk;

	return clk;
}


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