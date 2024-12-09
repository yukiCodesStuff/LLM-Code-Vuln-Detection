		return ERR_PTR(-EOPNOTSUPP);
	}

	/* Reject outgoing connection to device with same BD ADDR against
	 * CVE-2020-26555
	 */
	if (!bacmp(&hdev->bdaddr, dst)) {
		bt_dev_dbg(hdev, "Reject connection with same BD_ADDR %pMR\n",
			   dst);
		return ERR_PTR(-ECONNREFUSED);
	}

	acl = hci_conn_hash_lookup_ba(hdev, ACL_LINK, dst);
	if (!acl) {
		acl = hci_conn_add(hdev, ACL_LINK, dst, HCI_ROLE_MASTER);
		if (!acl)