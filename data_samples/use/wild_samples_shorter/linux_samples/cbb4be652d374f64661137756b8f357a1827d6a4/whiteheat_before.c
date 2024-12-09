static int  whiteheat_firmware_attach(struct usb_serial *serial);

/* function prototypes for the Connect Tech WhiteHEAT serial converter */
static int  whiteheat_attach(struct usb_serial *serial);
static void whiteheat_release(struct usb_serial *serial);
static int  whiteheat_port_probe(struct usb_serial_port *port);
static int  whiteheat_port_remove(struct usb_serial_port *port);
	.description =		"Connect Tech - WhiteHEAT",
	.id_table =		id_table_std,
	.num_ports =		4,
	.attach =		whiteheat_attach,
	.release =		whiteheat_release,
	.port_probe =		whiteheat_port_probe,
	.port_remove =		whiteheat_port_remove,
/*****************************************************************************
 * Connect Tech's White Heat serial driver functions
 *****************************************************************************/
static int whiteheat_attach(struct usb_serial *serial)
{
	struct usb_serial_port *command_port;
	struct whiteheat_command_private *command_info;