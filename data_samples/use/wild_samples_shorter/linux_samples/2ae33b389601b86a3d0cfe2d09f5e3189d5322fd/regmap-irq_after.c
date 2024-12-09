		if (ret < 0) {
			dev_err(map->dev, "IRQ thread failed to resume: %d\n",
				ret);
			pm_runtime_put(map->dev);
			return IRQ_NONE;
		}
	}
