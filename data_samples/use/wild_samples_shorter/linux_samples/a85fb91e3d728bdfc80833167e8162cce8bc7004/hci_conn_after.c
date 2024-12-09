			hdev->notify(hdev, HCI_NOTIFY_CONN_DEL);
	}

	debugfs_remove_recursive(conn->debugfs);

	hci_conn_del_sysfs(conn);

	hci_dev_put(hdev);
}

static void hci_acl_create_connection(struct hci_conn *conn)
{