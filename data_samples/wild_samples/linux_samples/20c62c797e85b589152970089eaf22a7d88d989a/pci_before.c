{
	struct ath10k *ar = pci_get_drvdata(pdev);
	struct ath10k_pci *ar_pci;

	ath10k_dbg(ar, ATH10K_DBG_PCI, "pci remove\n");

	if (!ar)
		return;

	ar_pci = ath10k_pci_priv(ar);

	if (!ar_pci)
		return;

	ath10k_core_unregister(ar);
	ath10k_pci_free_irq(ar);
	ath10k_pci_deinit_irq(ar);
	ath10k_pci_release_resource(ar);
	ath10k_pci_sleep_sync(ar);
	ath10k_pci_release(ar);
	ath10k_core_destroy(ar);
}

MODULE_DEVICE_TABLE(pci, ath10k_pci_id_table);

#ifdef CONFIG_PM

static int ath10k_pci_pm_suspend(struct device *dev)
{
	struct ath10k *ar = dev_get_drvdata(dev);
	int ret;

	if (test_bit(ATH10K_FW_FEATURE_WOWLAN_SUPPORT,
		     ar->running_fw->fw_file.fw_features))
		return 0;

	ret = ath10k_hif_suspend(ar);
	if (ret)
		ath10k_warn(ar, "failed to suspend hif: %d\n", ret);

	return ret;
}
{
	struct ath10k *ar = dev_get_drvdata(dev);
	int ret;

	if (test_bit(ATH10K_FW_FEATURE_WOWLAN_SUPPORT,
		     ar->running_fw->fw_file.fw_features))
		return 0;

	ret = ath10k_hif_suspend(ar);
	if (ret)
		ath10k_warn(ar, "failed to suspend hif: %d\n", ret);

	return ret;
}

static int ath10k_pci_pm_resume(struct device *dev)
{
	struct ath10k *ar = dev_get_drvdata(dev);
	int ret;

	if (test_bit(ATH10K_FW_FEATURE_WOWLAN_SUPPORT,
		     ar->running_fw->fw_file.fw_features))
		return 0;

	ret = ath10k_hif_resume(ar);
	if (ret)
		ath10k_warn(ar, "failed to resume hif: %d\n", ret);

	return ret;
}

static SIMPLE_DEV_PM_OPS(ath10k_pci_pm_ops,
			 ath10k_pci_pm_suspend,
			 ath10k_pci_pm_resume);
#endif

static struct pci_driver ath10k_pci_driver = {
	.name = "ath10k_pci",
	.id_table = ath10k_pci_id_table,
	.probe = ath10k_pci_probe,
	.remove = ath10k_pci_remove,
#ifdef CONFIG_PM
	.driver.pm = &ath10k_pci_pm_ops,
#endif
};

static int __init ath10k_pci_init(void)
{
	int ret;

	ret = pci_register_driver(&ath10k_pci_driver);
	if (ret)
		printk(KERN_ERR "failed to register ath10k pci driver: %d\n",
		       ret);

	ret = ath10k_ahb_init();
	if (ret)
		printk(KERN_ERR "ahb init failed: %d\n", ret);

	return ret;
}
{
	struct ath10k *ar = dev_get_drvdata(dev);
	int ret;

	if (test_bit(ATH10K_FW_FEATURE_WOWLAN_SUPPORT,
		     ar->running_fw->fw_file.fw_features))
		return 0;

	ret = ath10k_hif_resume(ar);
	if (ret)
		ath10k_warn(ar, "failed to resume hif: %d\n", ret);

	return ret;
}

static SIMPLE_DEV_PM_OPS(ath10k_pci_pm_ops,
			 ath10k_pci_pm_suspend,
			 ath10k_pci_pm_resume);
#endif

static struct pci_driver ath10k_pci_driver = {
	.name = "ath10k_pci",
	.id_table = ath10k_pci_id_table,
	.probe = ath10k_pci_probe,
	.remove = ath10k_pci_remove,
#ifdef CONFIG_PM
	.driver.pm = &ath10k_pci_pm_ops,
#endif
};

static int __init ath10k_pci_init(void)
{
	int ret;

	ret = pci_register_driver(&ath10k_pci_driver);
	if (ret)
		printk(KERN_ERR "failed to register ath10k pci driver: %d\n",
		       ret);

	ret = ath10k_ahb_init();
	if (ret)
		printk(KERN_ERR "ahb init failed: %d\n", ret);

	return ret;
}
module_init(ath10k_pci_init);

static void __exit ath10k_pci_exit(void)
{
	pci_unregister_driver(&ath10k_pci_driver);
	ath10k_ahb_exit();
}

module_exit(ath10k_pci_exit);

MODULE_AUTHOR("Qualcomm Atheros");
MODULE_DESCRIPTION("Driver support for Qualcomm Atheros 802.11ac WLAN PCIe/AHB devices");
MODULE_LICENSE("Dual BSD/GPL");

/* QCA988x 2.0 firmware files */
MODULE_FIRMWARE(QCA988X_HW_2_0_FW_DIR "/" ATH10K_FW_API2_FILE);
MODULE_FIRMWARE(QCA988X_HW_2_0_FW_DIR "/" ATH10K_FW_API3_FILE);
MODULE_FIRMWARE(QCA988X_HW_2_0_FW_DIR "/" ATH10K_FW_API4_FILE);
MODULE_FIRMWARE(QCA988X_HW_2_0_FW_DIR "/" ATH10K_FW_API5_FILE);
MODULE_FIRMWARE(QCA988X_HW_2_0_FW_DIR "/" QCA988X_HW_2_0_BOARD_DATA_FILE);
MODULE_FIRMWARE(QCA988X_HW_2_0_FW_DIR "/" ATH10K_BOARD_API2_FILE);

/* QCA9887 1.0 firmware files */
MODULE_FIRMWARE(QCA9887_HW_1_0_FW_DIR "/" ATH10K_FW_API5_FILE);
MODULE_FIRMWARE(QCA9887_HW_1_0_FW_DIR "/" QCA9887_HW_1_0_BOARD_DATA_FILE);
MODULE_FIRMWARE(QCA9887_HW_1_0_FW_DIR "/" ATH10K_BOARD_API2_FILE);

/* QCA6174 2.1 firmware files */
MODULE_FIRMWARE(QCA6174_HW_2_1_FW_DIR "/" ATH10K_FW_API4_FILE);
MODULE_FIRMWARE(QCA6174_HW_2_1_FW_DIR "/" ATH10K_FW_API5_FILE);
MODULE_FIRMWARE(QCA6174_HW_2_1_FW_DIR "/" QCA6174_HW_2_1_BOARD_DATA_FILE);
MODULE_FIRMWARE(QCA6174_HW_2_1_FW_DIR "/" ATH10K_BOARD_API2_FILE);

/* QCA6174 3.1 firmware files */
MODULE_FIRMWARE(QCA6174_HW_3_0_FW_DIR "/" ATH10K_FW_API4_FILE);
MODULE_FIRMWARE(QCA6174_HW_3_0_FW_DIR "/" ATH10K_FW_API5_FILE);
MODULE_FIRMWARE(QCA6174_HW_3_0_FW_DIR "/" ATH10K_FW_API6_FILE);
MODULE_FIRMWARE(QCA6174_HW_3_0_FW_DIR "/" QCA6174_HW_3_0_BOARD_DATA_FILE);
MODULE_FIRMWARE(QCA6174_HW_3_0_FW_DIR "/" ATH10K_BOARD_API2_FILE);

/* QCA9377 1.0 firmware files */
MODULE_FIRMWARE(QCA9377_HW_1_0_FW_DIR "/" ATH10K_FW_API5_FILE);
MODULE_FIRMWARE(QCA9377_HW_1_0_FW_DIR "/" QCA9377_HW_1_0_BOARD_DATA_FILE);