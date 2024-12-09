{
	struct hci_dev *hdev = to_hci_dev(dev);
	hci_release_dev(hdev);
	module_put(THIS_MODULE);
}