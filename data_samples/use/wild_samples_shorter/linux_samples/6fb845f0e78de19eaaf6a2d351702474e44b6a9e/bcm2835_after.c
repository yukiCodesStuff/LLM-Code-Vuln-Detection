
err:
	dev_dbg(dev, "%s -> err %d\n", __func__, ret);
	if (host->dma_chan_rxtx)
		dma_release_channel(host->dma_chan_rxtx);
	mmc_free_host(mmc);

	return ret;
}