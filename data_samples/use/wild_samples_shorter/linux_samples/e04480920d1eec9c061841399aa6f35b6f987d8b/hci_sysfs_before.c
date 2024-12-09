static void bt_host_release(struct device *dev)
{
	struct hci_dev *hdev = to_hci_dev(dev);
	kfree(hdev);
	module_put(THIS_MODULE);
}
