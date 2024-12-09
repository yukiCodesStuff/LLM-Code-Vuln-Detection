	if (host->irq <= 0) {
		dev_err(dev, "get IRQ failed\n");
		ret = -EINVAL;
		goto err;
	}

	ret = mmc_of_parse(mmc);
	if (ret)
		goto err;

	ret = bcm2835_add_host(host);
	if (ret)
		goto err;

	platform_set_drvdata(pdev, host);

	dev_dbg(dev, "%s -> OK\n", __func__);

	return 0;

err:
	dev_dbg(dev, "%s -> err %d\n", __func__, ret);
	if (host->dma_chan_rxtx)
		dma_release_channel(host->dma_chan_rxtx);
	mmc_free_host(mmc);

	return ret;
}

static int bcm2835_remove(struct platform_device *pdev)
{
	struct bcm2835_host *host = platform_get_drvdata(pdev);

	mmc_remove_host(host->mmc);

	writel(SDVDD_POWER_OFF, host->ioaddr + SDVDD);

	free_irq(host->irq, host);

	cancel_work_sync(&host->dma_work);
	cancel_delayed_work_sync(&host->timeout_work);

	if (host->dma_chan_rxtx)
		dma_release_channel(host->dma_chan_rxtx);

	mmc_free_host(host->mmc);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static const struct of_device_id bcm2835_match[] = {
	{ .compatible = "brcm,bcm2835-sdhost" },
	{ }
};
MODULE_DEVICE_TABLE(of, bcm2835_match);

static struct platform_driver bcm2835_driver = {
	.probe      = bcm2835_probe,
	.remove     = bcm2835_remove,
	.driver     = {
		.name		= "sdhost-bcm2835",
		.of_match_table	= bcm2835_match,
	},
};
module_platform_driver(bcm2835_driver);

MODULE_ALIAS("platform:sdhost-bcm2835");
MODULE_DESCRIPTION("BCM2835 SDHost driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Phil Elwell");