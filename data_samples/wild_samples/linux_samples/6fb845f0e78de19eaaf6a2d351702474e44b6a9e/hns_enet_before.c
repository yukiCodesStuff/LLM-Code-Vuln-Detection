		if (ret) {
			dev_err(dev, "register notifier fail!\n");
			goto out_notify_fail;
		}
		dev_dbg(dev, "has not handle, register notifier!\n");
	}

	return 0;

out_notify_fail:
	(void)cancel_work_sync(&priv->service_task);
out_read_prop_fail:
	free_netdev(ndev);
	return ret;
}

static int hns_nic_dev_remove(struct platform_device *pdev)
{
	struct net_device *ndev = platform_get_drvdata(pdev);
	struct hns_nic_priv *priv = netdev_priv(ndev);

	if (ndev->reg_state != NETREG_UNINITIALIZED)
		unregister_netdev(ndev);

	if (priv->ring_data)
		hns_nic_uninit_ring_data(priv);
	priv->ring_data = NULL;

	if (ndev->phydev)
		phy_disconnect(ndev->phydev);

	if (!IS_ERR_OR_NULL(priv->ae_handle))
		hnae_put_handle(priv->ae_handle);
	priv->ae_handle = NULL;
	if (priv->notifier_block.notifier_call)
		hnae_unregister_notifier(&priv->notifier_block);
	priv->notifier_block.notifier_call = NULL;

	set_bit(NIC_STATE_REMOVING, &priv->state);
	(void)cancel_work_sync(&priv->service_task);

	free_netdev(ndev);
	return 0;
}

static const struct of_device_id hns_enet_of_match[] = {
	{.compatible = "hisilicon,hns-nic-v1",},
	{.compatible = "hisilicon,hns-nic-v2",},
	{},
};

MODULE_DEVICE_TABLE(of, hns_enet_of_match);

static struct platform_driver hns_nic_dev_driver = {
	.driver = {
		.name = "hns-nic",
		.of_match_table = hns_enet_of_match,
		.acpi_match_table = ACPI_PTR(hns_enet_acpi_match),
	},
	.probe = hns_nic_dev_probe,
	.remove = hns_nic_dev_remove,
};

module_platform_driver(hns_nic_dev_driver);

MODULE_DESCRIPTION("HISILICON HNS Ethernet driver");
MODULE_AUTHOR("Hisilicon, Inc.");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:hns-nic");
{
	struct net_device *ndev = platform_get_drvdata(pdev);
	struct hns_nic_priv *priv = netdev_priv(ndev);

	if (ndev->reg_state != NETREG_UNINITIALIZED)
		unregister_netdev(ndev);

	if (priv->ring_data)
		hns_nic_uninit_ring_data(priv);
	priv->ring_data = NULL;

	if (ndev->phydev)
		phy_disconnect(ndev->phydev);

	if (!IS_ERR_OR_NULL(priv->ae_handle))
		hnae_put_handle(priv->ae_handle);
	priv->ae_handle = NULL;
	if (priv->notifier_block.notifier_call)
		hnae_unregister_notifier(&priv->notifier_block);
	priv->notifier_block.notifier_call = NULL;

	set_bit(NIC_STATE_REMOVING, &priv->state);
	(void)cancel_work_sync(&priv->service_task);

	free_netdev(ndev);
	return 0;
}

static const struct of_device_id hns_enet_of_match[] = {
	{.compatible = "hisilicon,hns-nic-v1",},
	{.compatible = "hisilicon,hns-nic-v2",},
	{},
};

MODULE_DEVICE_TABLE(of, hns_enet_of_match);

static struct platform_driver hns_nic_dev_driver = {
	.driver = {
		.name = "hns-nic",
		.of_match_table = hns_enet_of_match,
		.acpi_match_table = ACPI_PTR(hns_enet_acpi_match),
	},
	.probe = hns_nic_dev_probe,
	.remove = hns_nic_dev_remove,
};

module_platform_driver(hns_nic_dev_driver);

MODULE_DESCRIPTION("HISILICON HNS Ethernet driver");
MODULE_AUTHOR("Hisilicon, Inc.");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:hns-nic");