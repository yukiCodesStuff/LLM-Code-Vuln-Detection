{
	struct hci_ev_conn_request *ev = data;
	int mask = hdev->link_mode;
	struct inquiry_entry *ie;
	struct hci_conn *conn;
	__u8 flags = 0;

	bt_dev_dbg(hdev, "bdaddr %pMR type 0x%x", &ev->bdaddr, ev->link_type);

	/* Reject incoming connection from device with same BD ADDR against
	 * CVE-2020-26555
	 */
	if (!bacmp(&hdev->bdaddr, &ev->bdaddr))
	{
		bt_dev_dbg(hdev, "Reject connection with same BD_ADDR %pMR\n",
			   &ev->bdaddr);
		hci_reject_conn(hdev, &ev->bdaddr);
		return;
	}