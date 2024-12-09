	dw->regs = chip->regs;
	chip->dw = dw;

	pm_runtime_enable(chip->dev);
	pm_runtime_get_sync(chip->dev);

	dw_params = dma_read_byaddr(chip->regs, DW_PARAMS);
	autocfg = dw_params >> DW_PARAMS_EN & 0x1;
	}

	pm_runtime_put_sync_suspend(chip->dev);
	pm_runtime_disable(chip->dev);
	return 0;
}
EXPORT_SYMBOL_GPL(dw_dma_remove);
