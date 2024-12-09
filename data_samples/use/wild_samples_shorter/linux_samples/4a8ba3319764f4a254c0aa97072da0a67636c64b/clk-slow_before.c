
#define to_clk_sam9x5_slow(hw) container_of(hw, struct clk_sam9x5_slow, hw)


static int clk_slow_osc_prepare(struct clk_hw *hw)
{
	struct clk_slow_osc *osc = to_clk_slow_osc(hw);
	clk = clk_register(NULL, &slowck->hw);
	if (IS_ERR(clk))
		kfree(slowck);

	return clk;
}

	clk = clk_register(NULL, &slowck->hw);
	if (IS_ERR(clk))
		kfree(slowck);

	return clk;
}


	of_clk_add_provider(np, of_clk_src_simple_get, clk);
}