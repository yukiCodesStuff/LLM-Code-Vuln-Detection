			hdev->notify(hdev, HCI_NOTIFY_CONN_DEL);
	}

	hci_conn_del_sysfs(conn);

	debugfs_remove_recursive(conn->debugfs);

	hci_dev_put(hdev);

	hci_conn_put(conn);
}

static void hci_acl_create_connection(struct hci_conn *conn)
{