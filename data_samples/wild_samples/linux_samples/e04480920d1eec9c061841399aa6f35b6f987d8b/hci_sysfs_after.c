{
	struct hci_dev *hdev = to_hci_dev(dev);

	if (hci_dev_test_flag(hdev, HCI_UNREGISTER))
		hci_cleanup_dev(hdev);
	kfree(hdev);
	module_put(THIS_MODULE);
}