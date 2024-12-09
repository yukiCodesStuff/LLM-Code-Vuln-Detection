{
	unsigned long clk = clk_get_rate(priv->plat->stmmac_clk);

	if (!clk)
		return 0;

	return (usec * (clk / 1000000)) / 256;
}

{
	unsigned long clk = clk_get_rate(priv->plat->stmmac_clk);

	if (!clk)
		return 0;

	return (riwt * 256) / (clk / 1000000);
}
