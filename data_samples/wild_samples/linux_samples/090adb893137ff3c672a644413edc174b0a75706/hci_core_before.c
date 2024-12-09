{
	struct hci_dev *hdev = container_of(work, struct hci_dev, power_on);

	BT_DBG("%s", hdev->name);

	if (hci_dev_open(hdev->id) < 0)
		return;

	if (test_bit(HCI_AUTO_OFF, &hdev->dev_flags))
		queue_delayed_work(hdev->req_workqueue, &hdev->power_off,
				   HCI_AUTO_OFF_TIMEOUT);

	if (test_and_clear_bit(HCI_SETUP, &hdev->dev_flags))
		mgmt_index_added(hdev);
}