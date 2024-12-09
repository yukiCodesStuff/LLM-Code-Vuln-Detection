	if (!test_bit(HCI_UART_PROTO_READY, &hu->flags)) {
		percpu_up_read(&hu->proto_lock);
		return -EUNATCH;
	}

	hu->proto->enqueue(hu, skb);
	percpu_up_read(&hu->proto_lock);

	hci_uart_tx_wakeup(hu);

	return 0;
}

/* Flow control or un-flow control the device */
void hci_uart_set_flow_control(struct hci_uart *hu, bool enable)
{
	struct tty_struct *tty = hu->tty;
	struct ktermios ktermios;
	int status;
	unsigned int set = 0;
	unsigned int clear = 0;

	if (hu->serdev) {
		serdev_device_set_flow_control(hu->serdev, !enable);
		serdev_device_set_rts(hu->serdev, !enable);
		return;
	}