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

	hci_conn_hold(conn);
	conn->disc_timeout = HCI_DISCONN_TIMEOUT;
	hci_conn_drop(conn);

	set_bit(HCI_CONN_NEW_LINK_KEY, &conn->flags);
	conn_set_key(conn, ev->key_type, conn->pin_length);

	if (!hci_dev_test_flag(hdev, HCI_MGMT))
		goto unlock;

	key = hci_add_link_key(hdev, conn, &ev->bdaddr, ev->link_key,
			        ev->key_type, pin_len, &persistent);
	if (!key)
		goto unlock;

	/* Update connection information since adding the key will have
	 * fixed up the type in the case of changed combination keys.
	 */
	if (ev->key_type == HCI_LK_CHANGED_COMBINATION)
		conn_set_key(conn, key->type, key->pin_len);

	mgmt_new_link_key(hdev, key, persistent);

	/* Keep debug keys around only if the HCI_KEEP_DEBUG_KEYS flag
	 * is set. If it's not set simply remove the key from the kernel
	 * list (we've still notified user space about it but with
	 * store_hint being 0).
	 */
	if (key->type == HCI_LK_DEBUG_COMBINATION &&
	    !hci_dev_test_flag(hdev, HCI_KEEP_DEBUG_KEYS)) {
		list_del_rcu(&key->list);
		kfree_rcu(key, rcu);
		goto unlock;
	}