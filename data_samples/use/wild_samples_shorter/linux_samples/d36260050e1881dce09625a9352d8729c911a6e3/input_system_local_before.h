 */
struct rx_cfg_s {
	rx_mode_t			mode;	/* The HW config */
	mipi_port_ID_t		port;	/* The port ID to apply the control on */
	unsigned int		timeout;
	unsigned int		initcount;
	unsigned int		synccount;
	unsigned int		rxcount;