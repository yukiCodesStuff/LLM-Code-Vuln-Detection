	if (!conn)
		goto unlock;

	hci_conn_hold(conn);
	conn->disc_timeout = HCI_DISCONN_TIMEOUT;
	hci_conn_drop(conn);
