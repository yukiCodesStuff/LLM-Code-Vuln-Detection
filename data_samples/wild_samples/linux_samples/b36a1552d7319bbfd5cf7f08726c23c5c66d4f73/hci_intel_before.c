{
	struct intel_data *intel;

	BT_DBG("hu %p", hu);

	intel = kzalloc(sizeof(*intel), GFP_KERNEL);
	if (!intel)
		return -ENOMEM;

	skb_queue_head_init(&intel->txq);
	INIT_WORK(&intel->busy_work, intel_busy_work);

	intel->hu = hu;

	hu->priv = intel;

	if (!intel_set_power(hu, true))
		set_bit(STATE_BOOTING, &intel->flags);

	return 0;
}

static int intel_close(struct hci_uart *hu)
{
	struct intel_data *intel = hu->priv;

	BT_DBG("hu %p", hu);

	cancel_work_sync(&intel->busy_work);

	intel_set_power(hu, false);

	skb_queue_purge(&intel->txq);
	kfree_skb(intel->rx_skb);
	kfree(intel);

	hu->priv = NULL;
	return 0;
}

static int intel_flush(struct hci_uart *hu)
{
	struct intel_data *intel = hu->priv;

	BT_DBG("hu %p", hu);

	skb_queue_purge(&intel->txq);

	return 0;
}

static int inject_cmd_complete(struct hci_dev *hdev, __u16 opcode)
{
	struct sk_buff *skb;
	struct hci_event_hdr *hdr;
	struct hci_ev_cmd_complete *evt;

	skb = bt_skb_alloc(sizeof(*hdr) + sizeof(*evt) + 1, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;

	hdr = skb_put(skb, sizeof(*hdr));
	hdr->evt = HCI_EV_CMD_COMPLETE;
	hdr->plen = sizeof(*evt) + 1;

	evt = skb_put(skb, sizeof(*evt));
	evt->ncmd = 0x01;
	evt->opcode = cpu_to_le16(opcode);

	skb_put_u8(skb, 0x00);

	hci_skb_pkt_type(skb) = HCI_EVENT_PKT;

	return hci_recv_frame(hdev, skb);
}

static int intel_set_baudrate(struct hci_uart *hu, unsigned int speed)
{
	struct intel_data *intel = hu->priv;
	struct hci_dev *hdev = hu->hdev;
	u8 speed_cmd[] = { 0x06, 0xfc, 0x01, 0x00 };
	struct sk_buff *skb;
	int err;

	/* This can be the first command sent to the chip, check
	 * that the controller is ready.
	 */
	err = intel_wait_booting(hu);

	clear_bit(STATE_BOOTING, &intel->flags);

	/* In case of timeout, try to continue anyway */
	if (err && err != -ETIMEDOUT)
		return err;

	bt_dev_info(hdev, "Change controller speed to %d", speed);

	speed_cmd[3] = intel_convert_speed(speed);
	if (speed_cmd[3] == 0xff) {
		bt_dev_err(hdev, "Unsupported speed");
		return -EINVAL;
	}