		new_rate = clk->ops->determine_rate(clk->hw, rate,
						    &best_parent_rate,
						    &parent_hw);
		parent = parent_hw->clk;
	} else if (clk->ops->round_rate) {
		new_rate = clk->ops->round_rate(clk->hw, rate,
						&best_parent_rate);
	} else if (!parent || !(clk->flags & CLK_SET_RATE_PARENT)) {