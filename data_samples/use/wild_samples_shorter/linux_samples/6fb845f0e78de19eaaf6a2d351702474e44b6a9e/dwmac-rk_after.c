	}

	ret = phy_power_on(bsp_priv, true);
	if (ret) {
		gmac_clk_enable(bsp_priv, false);
		return ret;
	}

	pm_runtime_enable(dev);
	pm_runtime_get_sync(dev);
