	if (error) {
		dev_err(&func->dev, "failed to register ssb sdio bus,"
			" error %d\n", error);
		goto err_free_ssb;
	}
	sdio_set_drvdata(func, sdio);

	return 0;

err_free_ssb:
	kfree(sdio);
err_disable_func:
	sdio_claim_host(func);
	sdio_disable_func(func);
err_release_host:
	sdio_release_host(func);
out:
	return error;
}

static void b43_sdio_remove(struct sdio_func *func)
{
	struct b43_sdio *sdio = sdio_get_drvdata(func);

	ssb_bus_unregister(&sdio->ssb);
	sdio_claim_host(func);
	sdio_disable_func(func);
	sdio_release_host(func);
	kfree(sdio);
	sdio_set_drvdata(func, NULL);
}

static const struct sdio_device_id b43_sdio_ids[] = {
	{ SDIO_DEVICE(0x02d0, 0x044b) }, /* Nintendo Wii WLAN daughter card */
	{ SDIO_DEVICE(0x0092, 0x0004) }, /* C-guys, Inc. EW-CG1102GC */
	{ },
};

static struct sdio_driver b43_sdio_driver = {
	.name		= "b43-sdio",
	.id_table	= b43_sdio_ids,
	.probe		= b43_sdio_probe,
	.remove		= b43_sdio_remove,
};

int b43_sdio_init(void)
{
	return sdio_register_driver(&b43_sdio_driver);
}

void b43_sdio_exit(void)
{
	sdio_unregister_driver(&b43_sdio_driver);
}