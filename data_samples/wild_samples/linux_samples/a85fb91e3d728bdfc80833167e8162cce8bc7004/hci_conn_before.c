	} else {
		if (hdev->notify)
			hdev->notify(hdev, HCI_NOTIFY_CONN_DEL);
	}

	hci_conn_del_sysfs(conn);

	debugfs_remove_recursive(conn->debugfs);

	hci_dev_put(hdev);

	hci_conn_put(conn);
}

static void hci_acl_create_connection(struct hci_conn *conn)
{
	struct hci_dev *hdev = conn->hdev;
	struct inquiry_entry *ie;
	struct hci_cp_create_conn cp;

	BT_DBG("hcon %p", conn);

	/* Many controllers disallow HCI Create Connection while it is doing
	 * HCI Inquiry. So we cancel the Inquiry first before issuing HCI Create
	 * Connection. This may cause the MGMT discovering state to become false
	 * without user space's request but it is okay since the MGMT Discovery
	 * APIs do not promise that discovery should be done forever. Instead,
	 * the user space monitors the status of MGMT discovering and it may
	 * request for discovery again when this flag becomes false.
	 */
	if (test_bit(HCI_INQUIRY, &hdev->flags)) {
		/* Put this connection to "pending" state so that it will be
		 * executed after the inquiry cancel command complete event.
		 */
		conn->state = BT_CONNECT2;
		hci_send_cmd(hdev, HCI_OP_INQUIRY_CANCEL, 0, NULL);
		return;
	}

	conn->state = BT_CONNECT;
	conn->out = true;
	conn->role = HCI_ROLE_MASTER;

	conn->attempt++;

	conn->link_policy = hdev->link_policy;

	memset(&cp, 0, sizeof(cp));
	bacpy(&cp.bdaddr, &conn->dst);
	cp.pscan_rep_mode = 0x02;

	ie = hci_inquiry_cache_lookup(hdev, &conn->dst);
	if (ie) {
		if (inquiry_entry_age(ie) <= INQUIRY_ENTRY_AGE_MAX) {
			cp.pscan_rep_mode = ie->data.pscan_rep_mode;
			cp.pscan_mode     = ie->data.pscan_mode;
			cp.clock_offset   = ie->data.clock_offset |
					    cpu_to_le16(0x8000);
		}