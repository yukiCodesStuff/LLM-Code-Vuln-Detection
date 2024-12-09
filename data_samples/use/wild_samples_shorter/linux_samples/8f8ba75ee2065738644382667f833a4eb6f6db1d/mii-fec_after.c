	snprintf(new_bus->id, MII_BUS_ID_SIZE, "%x", res.start);

	fec->fecp = ioremap(res.start, resource_size(&res));
	if (!fec->fecp) {
		ret = -ENOMEM;
		goto out_fec;
	}

	if (get_bus_freq) {
		clock = get_bus_freq(ofdev->dev.of_node);
		if (!clock) {

	new_bus->phy_mask = ~0;
	new_bus->irq = kmalloc(sizeof(int) * PHY_MAX_ADDR, GFP_KERNEL);
	if (!new_bus->irq) {
		ret = -ENOMEM;
		goto out_unmap_regs;
	}

	new_bus->parent = &ofdev->dev;
	dev_set_drvdata(&ofdev->dev, new_bus);
