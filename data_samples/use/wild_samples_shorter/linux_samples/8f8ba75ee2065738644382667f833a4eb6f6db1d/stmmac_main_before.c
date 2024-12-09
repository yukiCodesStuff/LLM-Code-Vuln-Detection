		goto error_netdev_register;
	}

	priv->stmmac_clk = clk_get(priv->device, NULL);
	if (IS_ERR(priv->stmmac_clk)) {
		pr_warning("%s: warning: cannot get CSR clock\n", __func__);
		goto error_clk_get;
	}