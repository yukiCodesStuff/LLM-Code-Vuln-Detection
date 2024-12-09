 */
extern void receiver_port_enable(
	const rx_ID_t				ID,
	const enum mipi_port_id		port_ID,
	const bool					cnd);

/*! Flag if PORT[port_ID] of RECEIVER[ID] is enabled

 */
extern bool is_receiver_port_enabled(
	const rx_ID_t				ID,
	const enum mipi_port_id		port_ID);

/*! Enable the IRQ channels of PORT[port_ID] of RECEIVER[ID]

 \param	ID[in]				RECEIVER identifier
 */
extern void receiver_irq_enable(
	const rx_ID_t				ID,
	const enum mipi_port_id		port_ID,
	const rx_irq_info_t			irq_info);

/*! Return the IRQ status of PORT[port_ID] of RECEIVER[ID]

 */
extern rx_irq_info_t receiver_get_irq_info(
	const rx_ID_t				ID,
	const enum mipi_port_id		port_ID);

/*! Clear the IRQ status of PORT[port_ID] of RECEIVER[ID]

 \param	ID[in]				RECEIVER identifier
 */
extern void receiver_irq_clear(
	const rx_ID_t				ID,
	const enum mipi_port_id			port_ID,
	const rx_irq_info_t			irq_info);

/*! Write to a control register of INPUT_SYSTEM[ID]

 */
STORAGE_CLASS_INPUT_SYSTEM_H void receiver_port_reg_store(
	const rx_ID_t				ID,
	const enum mipi_port_id			port_ID,
	const hrt_address			reg,
	const hrt_data				value);

/*! Read from a control register PORT[port_ID] of of RECEIVER[ID]
 */
STORAGE_CLASS_INPUT_SYSTEM_H hrt_data receiver_port_reg_load(
	const rx_ID_t				ID,
	const enum mipi_port_id		port_ID,
	const hrt_address			reg);

/*! Write to a control register of SUB_SYSTEM[sub_ID] of INPUT_SYSTEM[ID]
