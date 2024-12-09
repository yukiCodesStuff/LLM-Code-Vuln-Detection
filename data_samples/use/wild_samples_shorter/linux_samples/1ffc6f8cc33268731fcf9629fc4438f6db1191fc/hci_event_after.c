
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

	mask |= hci_proto_connect_ind(hdev, &ev->bdaddr, ev->link_type,
				      &flags);

	if (!(mask & HCI_LM_ACCEPT)) {