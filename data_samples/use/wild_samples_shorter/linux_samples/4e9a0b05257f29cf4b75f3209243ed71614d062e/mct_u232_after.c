
static int mct_u232_port_probe(struct usb_serial_port *port)
{
	struct usb_serial *serial = port->serial;
	struct mct_u232_private *priv;

	/* check first to simplify error handling */
	if (!serial->port[1] || !serial->port[1]->interrupt_in_urb) {
		dev_err(&port->dev, "expected endpoint missing\n");
		return -ENODEV;
	}

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	/* Use second interrupt-in endpoint for reading. */
	priv->read_urb = serial->port[1]->interrupt_in_urb;
	priv->read_urb->context = port;

	spin_lock_init(&priv->lock);
