{
	struct hci_dev *hdev = conn->hdev;

	bt_dev_dbg(hdev, "conn %p", conn);

	conn->dev.type = &bt_link;
	conn->dev.class = &bt_class;
	conn->dev.parent = &hdev->dev;
{
	struct hci_dev *hdev = conn->hdev;

	bt_dev_dbg(hdev, "conn %p", conn);

	if (device_is_registered(&conn->dev))
		return;

	dev_set_name(&conn->dev, "%s:%d", hdev->name, conn->handle);

	if (device_add(&conn->dev) < 0)
		bt_dev_err(hdev, "failed to register connection device");
}

void hci_conn_del_sysfs(struct hci_conn *conn)
{
	struct hci_dev *hdev = conn->hdev;

	bt_dev_dbg(hdev, "conn %p", conn);

	if (!device_is_registered(&conn->dev)) {
		/* If device_add() has *not* succeeded, use *only* put_device()
		 * to drop the reference count.
		 */
		put_device(&conn->dev);
		return;
	}

	while (1) {
		struct device *dev;

		put_device(dev);
	}

	device_unregister(&conn->dev);
}

static void bt_host_release(struct device *dev)
{