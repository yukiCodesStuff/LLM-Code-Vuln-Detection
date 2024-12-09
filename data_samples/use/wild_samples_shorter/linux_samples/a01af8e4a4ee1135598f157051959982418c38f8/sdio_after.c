err_free_ssb:
	kfree(sdio);
err_disable_func:
	sdio_claim_host(func);
	sdio_disable_func(func);
err_release_host:
	sdio_release_host(func);
out: