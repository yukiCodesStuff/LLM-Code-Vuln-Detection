static void bt_host_release(struct device *dev)
{
	struct hci_dev *hdev = to_hci_dev(dev);
	hci_release_dev(hdev);
	module_put(THIS_MODULE);
}

static const struct device_type bt_host = {