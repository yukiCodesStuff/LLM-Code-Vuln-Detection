		return false;

	e = hci_inquiry_cache_lookup_resolve(hdev, BDADDR_ANY, NAME_NEEDED);
	if (hci_resolve_name(hdev, e) == 0) {
		e->name_state = NAME_PENDING;
		return true;
	}
		return;

	e = hci_inquiry_cache_lookup_resolve(hdev, bdaddr, NAME_PENDING);
	if (e) {
		e->name_state = NAME_KNOWN;
		list_del(&e->list);
		if (name)
			mgmt_remote_name(hdev, bdaddr, ACL_LINK, 0x00,
					 e->data.rssi, name, name_len);
	}

	if (hci_resolve_next_name(hdev))
		return;
		if (conn->type == ACL_LINK) {
			conn->state = BT_CONFIG;
			hci_conn_hold(conn);
			conn->disc_timeout = HCI_DISCONN_TIMEOUT;
		} else
			conn->state = BT_CONNECTED;

		hci_conn_hold_device(conn);