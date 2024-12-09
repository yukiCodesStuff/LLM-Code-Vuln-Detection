{
	struct hci_ev_link_key_notify *ev = data;
	struct hci_conn *conn;
	struct link_key *key;
	bool persistent;
	u8 pin_len = 0;

	bt_dev_dbg(hdev, "");

	hci_dev_lock(hdev);

	conn = hci_conn_hash_lookup_ba(hdev, ACL_LINK, &ev->bdaddr);
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