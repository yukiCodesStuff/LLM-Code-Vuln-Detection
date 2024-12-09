static int  whiteheat_firmware_attach(struct usb_serial *serial);

/* function prototypes for the Connect Tech WhiteHEAT serial converter */
static int whiteheat_probe(struct usb_serial *serial,
				const struct usb_device_id *id);
static int  whiteheat_attach(struct usb_serial *serial);
static void whiteheat_release(struct usb_serial *serial);
static int  whiteheat_port_probe(struct usb_serial_port *port);
static int  whiteheat_port_remove(struct usb_serial_port *port);
	.description =		"Connect Tech - WhiteHEAT",
	.id_table =		id_table_std,
	.num_ports =		4,
	.probe =		whiteheat_probe,
	.attach =		whiteheat_attach,
	.release =		whiteheat_release,
	.port_probe =		whiteheat_port_probe,
	.port_remove =		whiteheat_port_remove,
/*****************************************************************************
 * Connect Tech's White Heat serial driver functions
 *****************************************************************************/

static int whiteheat_probe(struct usb_serial *serial,
				const struct usb_device_id *id)
{
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	size_t num_bulk_in = 0;
	size_t num_bulk_out = 0;
	size_t min_num_bulk;
	unsigned int i;

	iface_desc = serial->interface->cur_altsetting;

	for (i = 0; i < iface_desc->desc.bNumEndpoints; i++) {
		endpoint = &iface_desc->endpoint[i].desc;
		if (usb_endpoint_is_bulk_in(endpoint))
			++num_bulk_in;
		if (usb_endpoint_is_bulk_out(endpoint))
			++num_bulk_out;
	}

	min_num_bulk = COMMAND_PORT + 1;
	if (num_bulk_in < min_num_bulk || num_bulk_out < min_num_bulk)
		return -ENODEV;

	return 0;
}

static int whiteheat_attach(struct usb_serial *serial)
{
	struct usb_serial_port *command_port;
	struct whiteheat_command_private *command_info;