		goto error_netdev_register;
	}

	priv->stmmac_clk = clk_get(priv->device, STMMAC_RESOURCE_NAME);
	if (IS_ERR(priv->stmmac_clk)) {
		pr_warning("%s: warning: cannot get CSR clock\n", __func__);
		goto error_clk_get;
	}