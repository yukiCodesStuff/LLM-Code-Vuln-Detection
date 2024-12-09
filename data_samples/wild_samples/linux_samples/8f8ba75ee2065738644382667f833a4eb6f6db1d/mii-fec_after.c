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
	if (!fec->fecp) {
		ret = -ENOMEM;
		goto out_fec;
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
	if (!new_bus->irq) {
		ret = -ENOMEM;
		goto out_unmap_regs;
	}