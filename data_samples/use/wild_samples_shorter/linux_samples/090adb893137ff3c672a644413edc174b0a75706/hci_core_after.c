static void hci_power_on(struct work_struct *work)
{
	struct hci_dev *hdev = container_of(work, struct hci_dev, power_on);
	int err;

	BT_DBG("%s", hdev->name);

	err = hci_dev_open(hdev->id);
	if (err < 0) {
		mgmt_set_powered_failed(hdev, err);
		return;
	}

	if (test_bit(HCI_AUTO_OFF, &hdev->dev_flags))
		queue_delayed_work(hdev->req_workqueue, &hdev->power_off,
				   HCI_AUTO_OFF_TIMEOUT);