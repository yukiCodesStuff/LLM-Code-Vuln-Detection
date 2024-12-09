
err:
	dev_dbg(dev, "%s -> err %d\n", __func__, ret);
	mmc_free_host(mmc);

	return ret;
}