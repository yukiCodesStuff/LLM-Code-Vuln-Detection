	if (!conn)
		goto unlock;

	/* Ignore NULL link key against CVE-2020-26555 */
	if (!memcmp(ev->link_key, ZERO_KEY, HCI_LINK_KEY_SIZE)) {
		bt_dev_dbg(hdev, "Ignore NULL link key (ZERO KEY) for %pMR",
			   &ev->bdaddr);
		hci_disconnect(conn, HCI_ERROR_AUTH_FAILURE);
		hci_conn_drop(conn);
		goto unlock;
	}

	hci_conn_hold(conn);
	conn->disc_timeout = HCI_DISCONN_TIMEOUT;
	hci_conn_drop(conn);
