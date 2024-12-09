/* Unregister HCI device */
void hci_unregister_dev(struct hci_dev *hdev)
{
	BT_DBG("%p name %s bus %d", hdev, hdev->name, hdev->bus);

	hci_dev_set_flag(hdev, HCI_UNREGISTER);

	write_lock(&hci_dev_list_lock);
	list_del(&hdev->list);
	write_unlock(&hci_dev_list_lock);

	}

	device_del(&hdev->dev);
	/* Actual cleanup is deferred until hci_cleanup_dev(). */
	hci_dev_put(hdev);
}
EXPORT_SYMBOL(hci_unregister_dev);

/* Cleanup HCI device */
void hci_cleanup_dev(struct hci_dev *hdev)
{
	debugfs_remove_recursive(hdev->debugfs);
	kfree_const(hdev->hw_info);
	kfree_const(hdev->fw_info);

	hci_blocked_keys_clear(hdev);
	hci_dev_unlock(hdev);

	ida_simple_remove(&hci_index_ida, hdev->id);
}

/* Suspend HCI device */
int hci_suspend_dev(struct hci_dev *hdev)
{