	struct usb_serial *serial = port->serial;
	struct cypress_private *priv;

	if (!port->interrupt_out_urb || !port->interrupt_in_urb) {
		dev_err(&port->dev, "required endpoint is missing\n");
		return -ENODEV;
	}

	priv = kzalloc(sizeof(struct cypress_private), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

		cypress_set_termios(tty, port, &priv->tmp_termios);

	/* setup the port and start reading from the device */
	usb_fill_int_urb(port->interrupt_in_urb, serial->dev,
		usb_rcvintpipe(serial->dev, port->interrupt_in_endpointAddress),
		port->interrupt_in_urb->transfer_buffer,
		port->interrupt_in_urb->transfer_buffer_length,