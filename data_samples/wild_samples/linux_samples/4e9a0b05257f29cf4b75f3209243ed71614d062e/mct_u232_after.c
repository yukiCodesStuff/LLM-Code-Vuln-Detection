{
	/* Translate Control Line states */
	if (msr & MCT_U232_MSR_DSR)
		*control_state |=  TIOCM_DSR;
	else
		*control_state &= ~TIOCM_DSR;
	if (msr & MCT_U232_MSR_CTS)
		*control_state |=  TIOCM_CTS;
	else
		*control_state &= ~TIOCM_CTS;
	if (msr & MCT_U232_MSR_RI)
		*control_state |=  TIOCM_RI;
	else
		*control_state &= ~TIOCM_RI;
	if (msr & MCT_U232_MSR_CD)
		*control_state |=  TIOCM_CD;
	else
		*control_state &= ~TIOCM_CD;
	dev_dbg(&port->dev, "msr_to_state: msr=0x%x ==> state=0x%x\n", msr, *control_state);
} /* mct_u232_msr_to_state */

/*
 * Driver's tty interface functions
 */

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

	usb_set_serial_port_data(port, priv);

	return 0;
}