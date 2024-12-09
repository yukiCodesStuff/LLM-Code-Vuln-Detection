	return 0;
}

/* Check the underlying device or tty has flow control support */
bool hci_uart_has_flow_control(struct hci_uart *hu)
{
	/* serdev nodes check if the needed operations are present */
	if (hu->serdev)
		return true;

	if (hu->tty->driver->ops->tiocmget && hu->tty->driver->ops->tiocmset)
		return true;

	return false;
}

/* Flow control or un-flow control the device */
void hci_uart_set_flow_control(struct hci_uart *hu, bool enable)
{
	struct tty_struct *tty = hu->tty;