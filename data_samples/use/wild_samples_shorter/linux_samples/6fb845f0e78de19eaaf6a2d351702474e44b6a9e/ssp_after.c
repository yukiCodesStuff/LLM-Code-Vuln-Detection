	if (ssp == NULL)
		return -ENODEV;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(res->start, resource_size(res));

	clk_put(ssp->clk);
	list_del(&ssp->node);
	mutex_unlock(&ssp_lock);

	return 0;
}

static const struct platform_device_id ssp_id_table[] = {