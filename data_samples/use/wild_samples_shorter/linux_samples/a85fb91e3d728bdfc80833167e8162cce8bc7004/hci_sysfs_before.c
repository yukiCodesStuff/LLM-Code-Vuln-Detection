{
	struct hci_dev *hdev = conn->hdev;

	BT_DBG("conn %p", conn);

	conn->dev.type = &bt_link;
	conn->dev.class = &bt_class;
	conn->dev.parent = &hdev->dev;
{
	struct hci_dev *hdev = conn->hdev;

	BT_DBG("conn %p", conn);

	if (device_is_registered(&conn->dev))
		return;

	dev_set_name(&conn->dev, "%s:%d", hdev->name, conn->handle);

	if (device_add(&conn->dev) < 0) {
		bt_dev_err(hdev, "failed to register connection device");
		return;
	}

	hci_dev_hold(hdev);
}

void hci_conn_del_sysfs(struct hci_conn *conn)
{
	struct hci_dev *hdev = conn->hdev;

	if (!device_is_registered(&conn->dev))
		return;

	while (1) {
		struct device *dev;

		put_device(dev);
	}

	device_del(&conn->dev);

	hci_dev_put(hdev);
}

static void bt_host_release(struct device *dev)
{