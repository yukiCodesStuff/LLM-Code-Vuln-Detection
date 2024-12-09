 */
struct rx_cfg_s {
	rx_mode_t			mode;	/* The HW config */
	enum mipi_port_id		port;	/* The port ID to apply the control on */
	unsigned int		timeout;
	unsigned int		initcount;
	unsigned int		synccount;
	unsigned int		rxcount;