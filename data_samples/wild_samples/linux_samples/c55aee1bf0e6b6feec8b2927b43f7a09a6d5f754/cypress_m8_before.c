{
	struct usb_serial *serial = port->serial;
	struct cypress_private *priv;

	priv = kzalloc(sizeof(struct cypress_private), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->comm_is_ok = !0;
	spin_lock_init(&priv->lock);
	if (kfifo_alloc(&priv->write_fifo, CYPRESS_BUF_SIZE, GFP_KERNEL)) {
		kfree(priv);
		return -ENOMEM;
	}
{
	struct cypress_private *priv = usb_get_serial_port_data(port);
	struct usb_serial *serial = port->serial;
	unsigned long flags;
	int result = 0;

	if (!priv->comm_is_ok)
		return -EIO;

	/* clear halts before open */
	usb_clear_halt(serial->dev, 0x81);
	usb_clear_halt(serial->dev, 0x02);

	spin_lock_irqsave(&priv->lock, flags);
	/* reset read/write statistics */
	priv->bytes_in = 0;
	priv->bytes_out = 0;
	priv->cmd_count = 0;
	priv->rx_flags = 0;
	spin_unlock_irqrestore(&priv->lock, flags);

	/* Set termios */
	cypress_send(port);

	if (tty)
		cypress_set_termios(tty, port, &priv->tmp_termios);

	/* setup the port and start reading from the device */
	if (!port->interrupt_in_urb) {
		dev_err(&port->dev, "%s - interrupt_in_urb is empty!\n",
			__func__);
		return -1;
	}