{
	const struct of_device_id *match;
	struct resource res;
	struct mii_bus *new_bus;
	struct fec_info *fec;
	int (*get_bus_freq)(struct device_node *);
	int ret = -ENOMEM, clock, speed;

	match = of_match_device(fs_enet_mdio_fec_match, &ofdev->dev);
	if (!match)
		return -EINVAL;
	get_bus_freq = match->data;

	new_bus = mdiobus_alloc();
	if (!new_bus)
		goto out;

	fec = kzalloc(sizeof(struct fec_info), GFP_KERNEL);
	if (!fec)
		goto out_mii;

	new_bus->priv = fec;
	new_bus->name = "FEC MII Bus";
	new_bus->read = &fs_enet_fec_mii_read;
	new_bus->write = &fs_enet_fec_mii_write;
	new_bus->reset = &fs_enet_fec_mii_reset;

	ret = of_address_to_resource(ofdev->dev.of_node, 0, &res);
	if (ret)
		goto out_res;

	snprintf(new_bus->id, MII_BUS_ID_SIZE, "%x", res.start);

	fec->fecp = ioremap(res.start, resource_size(&res));
	if (!fec->fecp)
		goto out_fec;

	if (get_bus_freq) {
		clock = get_bus_freq(ofdev->dev.of_node);
		if (!clock) {
			/* Use maximum divider if clock is unknown */
			dev_warn(&ofdev->dev, "could not determine IPS clock\n");
			clock = 0x3F * 5000000;
		}
	} else
		clock = ppc_proc_freq;

	/*
	 * Scale for a MII clock <= 2.5 MHz
	 * Note that only 6 bits (25:30) are available for MII speed.
	 */
	speed = (clock + 4999999) / 5000000;
	if (speed > 0x3F) {
		speed = 0x3F;
		dev_err(&ofdev->dev,
			"MII clock (%d Hz) exceeds max (2.5 MHz)\n",
			clock / speed);
	}

	fec->mii_speed = speed << 1;

	setbits32(&fec->fecp->fec_r_cntrl, FEC_RCNTRL_MII_MODE);
	setbits32(&fec->fecp->fec_ecntrl, FEC_ECNTRL_PINMUX |
	                                  FEC_ECNTRL_ETHER_EN);
	out_be32(&fec->fecp->fec_ievent, FEC_ENET_MII);
	clrsetbits_be32(&fec->fecp->fec_mii_speed, 0x7E, fec->mii_speed);

	new_bus->phy_mask = ~0;
	new_bus->irq = kmalloc(sizeof(int) * PHY_MAX_ADDR, GFP_KERNEL);
	if (!new_bus->irq)
		goto out_unmap_regs;

	new_bus->parent = &ofdev->dev;
	dev_set_drvdata(&ofdev->dev, new_bus);

	ret = of_mdiobus_register(new_bus, ofdev->dev.of_node);
	if (ret)
		goto out_free_irqs;

	return 0;

out_free_irqs:
	dev_set_drvdata(&ofdev->dev, NULL);
	kfree(new_bus->irq);
out_unmap_regs:
	iounmap(fec->fecp);
out_res:
out_fec:
	kfree(fec);
out_mii:
	mdiobus_free(new_bus);
out:
	return ret;
}
	if (speed > 0x3F) {
		speed = 0x3F;
		dev_err(&ofdev->dev,
			"MII clock (%d Hz) exceeds max (2.5 MHz)\n",
			clock / speed);
	}

	fec->mii_speed = speed << 1;

	setbits32(&fec->fecp->fec_r_cntrl, FEC_RCNTRL_MII_MODE);
	setbits32(&fec->fecp->fec_ecntrl, FEC_ECNTRL_PINMUX |
	                                  FEC_ECNTRL_ETHER_EN);
	out_be32(&fec->fecp->fec_ievent, FEC_ENET_MII);
	clrsetbits_be32(&fec->fecp->fec_mii_speed, 0x7E, fec->mii_speed);

	new_bus->phy_mask = ~0;
	new_bus->irq = kmalloc(sizeof(int) * PHY_MAX_ADDR, GFP_KERNEL);
	if (!new_bus->irq)
		goto out_unmap_regs;

	new_bus->parent = &ofdev->dev;
	dev_set_drvdata(&ofdev->dev, new_bus);

	ret = of_mdiobus_register(new_bus, ofdev->dev.of_node);
	if (ret)
		goto out_free_irqs;

	return 0;

out_free_irqs:
	dev_set_drvdata(&ofdev->dev, NULL);
	kfree(new_bus->irq);
out_unmap_regs:
	iounmap(fec->fecp);
out_res:
out_fec:
	kfree(fec);
out_mii:
	mdiobus_free(new_bus);
out:
	return ret;
}

static int fs_enet_mdio_remove(struct platform_device *ofdev)
{
	struct mii_bus *bus = dev_get_drvdata(&ofdev->dev);
	struct fec_info *fec = bus->priv;

	mdiobus_unregister(bus);
	dev_set_drvdata(&ofdev->dev, NULL);
	kfree(bus->irq);
	iounmap(fec->fecp);
	kfree(fec);
	mdiobus_free(bus);

	return 0;
}

static struct of_device_id fs_enet_mdio_fec_match[] = {
	{
		.compatible = "fsl,pq1-fec-mdio",
	},
#if defined(CONFIG_PPC_MPC512x)
	{
		.compatible = "fsl,mpc5121-fec-mdio",
		.data = mpc5xxx_get_bus_frequency,
	},
#endif
	{},
};
MODULE_DEVICE_TABLE(of, fs_enet_mdio_fec_match);

static struct platform_driver fs_enet_fec_mdio_driver = {
	.driver = {
		.name = "fsl-fec-mdio",
		.owner = THIS_MODULE,
		.of_match_table = fs_enet_mdio_fec_match,
	},
	.probe = fs_enet_mdio_probe,
	.remove = fs_enet_mdio_remove,
};

module_platform_driver(fs_enet_fec_mdio_driver);