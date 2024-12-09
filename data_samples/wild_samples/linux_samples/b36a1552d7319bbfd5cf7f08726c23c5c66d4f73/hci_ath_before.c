{
	struct ath_struct *ath;

	BT_DBG("hu %p", hu);

	ath = kzalloc(sizeof(*ath), GFP_KERNEL);
	if (!ath)
		return -ENOMEM;

	skb_queue_head_init(&ath->txq);

	hu->priv = ath;
	ath->hu = hu;

	INIT_WORK(&ath->ctxtsw, ath_hci_uart_work);

	return 0;
}

static int ath_close(struct hci_uart *hu)
{
	struct ath_struct *ath = hu->priv;

	BT_DBG("hu %p", hu);

	skb_queue_purge(&ath->txq);

	kfree_skb(ath->rx_skb);

	cancel_work_sync(&ath->ctxtsw);

	hu->priv = NULL;
	kfree(ath);

	return 0;
}

static int ath_flush(struct hci_uart *hu)
{
	struct ath_struct *ath = hu->priv;

	BT_DBG("hu %p", hu);

	skb_queue_purge(&ath->txq);

	return 0;
}

static int ath_vendor_cmd(struct hci_dev *hdev, uint8_t opcode, uint16_t index,
			  const void *data, size_t dlen)
{
	struct sk_buff *skb;
	struct ath_vendor_cmd cmd;

	if (dlen > sizeof(cmd.data))
		return -EINVAL;

	cmd.opcode = opcode;
	cmd.index = cpu_to_le16(index);
	cmd.len = dlen;
	memcpy(cmd.data, data, dlen);

	skb = __hci_cmd_sync(hdev, 0xfc0b, dlen + 4, &cmd, HCI_INIT_TIMEOUT);
	if (IS_ERR(skb))
		return PTR_ERR(skb);
	kfree_skb(skb);

	return 0;
}

static int ath_set_bdaddr(struct hci_dev *hdev, const bdaddr_t *bdaddr)
{
	return ath_vendor_cmd(hdev, OP_WRITE_TAG, INDEX_BDADDR, bdaddr,
			      sizeof(*bdaddr));
}

static int ath_setup(struct hci_uart *hu)
{
	BT_DBG("hu %p", hu);

	hu->hdev->set_bdaddr = ath_set_bdaddr;

	return 0;
}

static const struct h4_recv_pkt ath_recv_pkts[] = {
	{ H4_RECV_ACL,   .recv = hci_recv_frame },
	{ H4_RECV_SCO,   .recv = hci_recv_frame },
	{ H4_RECV_EVENT, .recv = hci_recv_frame },
};

static int ath_recv(struct hci_uart *hu, const void *data, int count)
{
	struct ath_struct *ath = hu->priv;

	ath->rx_skb = h4_recv_buf(hu->hdev, ath->rx_skb, data, count,
				  ath_recv_pkts, ARRAY_SIZE(ath_recv_pkts));
	if (IS_ERR(ath->rx_skb)) {
		int err = PTR_ERR(ath->rx_skb);
		bt_dev_err(hu->hdev, "Frame reassembly failed (%d)", err);
		ath->rx_skb = NULL;
		return err;
	}

	return count;
}