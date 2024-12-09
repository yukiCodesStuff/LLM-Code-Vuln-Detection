			return PTR_ERR(tcon->sclk0);
		}
	}
	clk_prepare_enable(tcon->sclk0);

	if (tcon->quirks->has_channel_1) {
		tcon->sclk1 = devm_clk_get(dev, "tcon-ch1");
		if (IS_ERR(tcon->sclk1)) {

static void sun4i_tcon_free_clocks(struct sun4i_tcon *tcon)
{
	clk_disable_unprepare(tcon->sclk0);
	clk_disable_unprepare(tcon->clk);
}

static int sun4i_tcon_init_irq(struct device *dev,