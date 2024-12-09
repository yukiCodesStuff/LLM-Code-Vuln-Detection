{
	struct mii_bus *new_bus;
	struct bb_info *bitbang;
	int ret = -ENOMEM;

	bitbang = kzalloc(sizeof(struct bb_info), GFP_KERNEL);
	if (!bitbang)
		goto out;

	bitbang->ctrl.ops = &bb_ops;

	new_bus = alloc_mdio_bitbang(&bitbang->ctrl);
	if (!new_bus)
		goto out_free_priv;

	new_bus->name = "CPM2 Bitbanged MII",

	ret = fs_mii_bitbang_init(new_bus, ofdev->dev.of_node);
	if (ret)
		goto out_free_bus;

	new_bus->phy_mask = ~0;
	new_bus->irq = kmalloc(sizeof(int) * PHY_MAX_ADDR, GFP_KERNEL);
	if (!new_bus->irq) {
		ret = -ENOMEM;
		goto out_unmap_regs;
	}