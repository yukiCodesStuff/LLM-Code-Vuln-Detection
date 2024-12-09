	if (!hci_dev_test_flag(hdev, HCI_BREDR_ENABLED)) {
		if (lmp_bredr_capable(hdev))
			return ERR_PTR(-ECONNREFUSED);

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
			return ERR_PTR(-ENOMEM);
	}

	hci_conn_hold(acl);

	acl->conn_reason = conn_reason;
	if (acl->state == BT_OPEN || acl->state == BT_CLOSED) {
		acl->sec_level = BT_SECURITY_LOW;
		acl->pending_sec_level = sec_level;
		acl->auth_type = auth_type;
		hci_acl_create_connection(acl);
	}

	return acl;
}

static struct hci_link *hci_conn_link(struct hci_conn *parent,
				      struct hci_conn *conn)
{
	struct hci_dev *hdev = parent->hdev;
	struct hci_link *link;

	bt_dev_dbg(hdev, "parent %p hcon %p", parent, conn);

	if (conn->link)
		return conn->link;

	if (conn->parent)
		return NULL;

	link = kzalloc(sizeof(*link), GFP_KERNEL);
	if (!link)
		return NULL;

	link->conn = hci_conn_hold(conn);
	conn->link = link;
	conn->parent = hci_conn_get(parent);

	/* Use list_add_tail_rcu append to the list */
	list_add_tail_rcu(&link->list, &parent->link_list);

	return link;
}

struct hci_conn *hci_connect_sco(struct hci_dev *hdev, int type, bdaddr_t *dst,
				 __u16 setting, struct bt_codec *codec)
{
	struct hci_conn *acl;
	struct hci_conn *sco;
	struct hci_link *link;

	acl = hci_connect_acl(hdev, dst, BT_SECURITY_LOW, HCI_AT_NO_BONDING,
			      CONN_REASON_SCO_CONNECT);
	if (IS_ERR(acl))
		return acl;

	sco = hci_conn_hash_lookup_ba(hdev, type, dst);
	if (!sco) {
		sco = hci_conn_add(hdev, type, dst, HCI_ROLE_MASTER);
		if (!sco) {
			hci_conn_drop(acl);
			return ERR_PTR(-ENOMEM);
		}