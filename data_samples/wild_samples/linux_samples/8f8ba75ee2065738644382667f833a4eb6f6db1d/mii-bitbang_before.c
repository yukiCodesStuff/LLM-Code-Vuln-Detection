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
	iounmap(bitbang->dir);
out_free_bus:
	free_mdio_bitbang(new_bus);
out_free_priv:
	kfree(bitbang);
out:
	return ret;
}

static int fs_enet_mdio_remove(struct platform_device *ofdev)
{
	struct mii_bus *bus = dev_get_drvdata(&ofdev->dev);
	struct bb_info *bitbang = bus->priv;

	mdiobus_unregister(bus);
	dev_set_drvdata(&ofdev->dev, NULL);
	kfree(bus->irq);
	free_mdio_bitbang(bus);
	iounmap(bitbang->dir);
	kfree(bitbang);

	return 0;
}

static struct of_device_id fs_enet_mdio_bb_match[] = {
	{
		.compatible = "fsl,cpm2-mdio-bitbang",
	},
	{},
};
MODULE_DEVICE_TABLE(of, fs_enet_mdio_bb_match);

static struct platform_driver fs_enet_bb_mdio_driver = {
	.driver = {
		.name = "fsl-bb-mdio",
		.owner = THIS_MODULE,
		.of_match_table = fs_enet_mdio_bb_match,
	},
	.probe = fs_enet_mdio_probe,
	.remove = fs_enet_mdio_remove,
};

module_platform_driver(fs_enet_bb_mdio_driver);