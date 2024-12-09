 * that occurred.
 */
void
ia_css_rx_port_get_irq_info(enum mipi_port_id port, unsigned int *irq_bits);

/* @brief Clear CSI receiver error info.
 *
 * @param[in] irq_bits	The bits that should be cleared from the CSI receiver
 * error bits get overwritten.
 */
void
ia_css_rx_port_clear_irq_info(enum mipi_port_id port, unsigned int irq_bits);

/* @brief Enable or disable specific interrupts.
 *
 * @param[in] type	The interrupt type that will be enabled/disabled.