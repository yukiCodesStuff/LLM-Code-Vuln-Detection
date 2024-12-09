struct ia_css_irq {
	enum ia_css_irq_info type; /** Interrupt type. */
	unsigned int sw_irq_0_val; /** In case of SW interrupt 0, value. */
	unsigned int sw_irq_1_val; /** In case of SW interrupt 1, value. */
	unsigned int sw_irq_2_val; /** In case of SW interrupt 2, value. */
	struct ia_css_pipe *pipe;
	/** The image pipe that generated the interrupt. */
};

/* @brief Obtain interrupt information.
 *
 * @param[out] info	Pointer to the interrupt info. The interrupt
 *			information wil be written to this info.
 * @return		If an error is encountered during the interrupt info
 *			and no interrupt could be translated successfully, this
 *			will return IA_CSS_INTERNAL_ERROR. Otherwise
 *			IA_CSS_SUCCESS.
 *
 * This function is expected to be executed after an interrupt has been sent
 * to the IA from the CSS. This function returns information about the interrupt
 * which is needed by the IA code to properly handle the interrupt. This
 * information includes the image pipe, buffer type etc.
 */
enum ia_css_err
ia_css_irq_translate(unsigned int *info);

/* @brief Get CSI receiver error info.
 *
 * @param[out] irq_bits	Pointer to the interrupt bits. The interrupt
 *			bits will be written this info.
 *			This will be the error bits that are enabled in the CSI
 *			receiver error register.
 * @return	None
 *
 * This function should be used whenever a CSI receiver error interrupt is
 * generated. It provides the detailed information (bits) on the exact error
 * that occurred.
 *
 *@deprecated {this function is DEPRECATED since it only works on CSI port 1.
 * Use the function below instead and specify the appropriate port.}
 */
void
ia_css_rx_get_irq_info(unsigned int *irq_bits);

/* @brief Get CSI receiver error info.
 *
 * @param[in]  port     Input port identifier.
 * @param[out] irq_bits	Pointer to the interrupt bits. The interrupt
 *			bits will be written this info.
 *			This will be the error bits that are enabled in the CSI
 *			receiver error register.
 * @return	None
 *
 * This function should be used whenever a CSI receiver error interrupt is
 * generated. It provides the detailed information (bits) on the exact error
 * that occurred.
 */
void
ia_css_rx_port_get_irq_info(enum ia_css_csi2_port port, unsigned int *irq_bits);

/* @brief Clear CSI receiver error info.
 *
 * @param[in] irq_bits	The bits that should be cleared from the CSI receiver
 *			interrupt bits register.
 * @return	None
 *
 * This function should be called after ia_css_rx_get_irq_info has been called
 * and the error bits have been interpreted. It is advised to use the return
 * value of that function as the argument to this function to make sure no new
 * error bits get overwritten.
 *
 * @deprecated{this function is DEPRECATED since it only works on CSI port 1.
 * Use the function below instead and specify the appropriate port.}
 */
void
ia_css_rx_clear_irq_info(unsigned int irq_bits);

/* @brief Clear CSI receiver error info.
 *
 * @param[in] port      Input port identifier.
 * @param[in] irq_bits	The bits that should be cleared from the CSI receiver
 *			interrupt bits register.
 * @return	None
 *
 * This function should be called after ia_css_rx_get_irq_info has been called
 * and the error bits have been interpreted. It is advised to use the return
 * value of that function as the argument to this function to make sure no new
 * error bits get overwritten.
 */
void
ia_css_rx_port_clear_irq_info(enum ia_css_csi2_port port, unsigned int irq_bits);

/* @brief Enable or disable specific interrupts.
 *
 * @param[in] type	The interrupt type that will be enabled/disabled.
 * @param[in] enable	enable or disable.
 * @return		Returns IA_CSS_INTERNAL_ERROR if this interrupt
 *			type cannot be enabled/disabled which is true for
 *			CSS internal interrupts. Otherwise returns
 *			IA_CSS_SUCCESS.
 */
enum ia_css_err
ia_css_irq_enable(enum ia_css_irq_info type, bool enable);

#endif /* __IA_CSS_IRQ_H */
struct ia_css_irq {
	enum ia_css_irq_info type; /** Interrupt type. */
	unsigned int sw_irq_0_val; /** In case of SW interrupt 0, value. */
	unsigned int sw_irq_1_val; /** In case of SW interrupt 1, value. */
	unsigned int sw_irq_2_val; /** In case of SW interrupt 2, value. */
	struct ia_css_pipe *pipe;
	/** The image pipe that generated the interrupt. */
};

/* @brief Obtain interrupt information.
 *
 * @param[out] info	Pointer to the interrupt info. The interrupt
 *			information wil be written to this info.
 * @return		If an error is encountered during the interrupt info
 *			and no interrupt could be translated successfully, this
 *			will return IA_CSS_INTERNAL_ERROR. Otherwise
 *			IA_CSS_SUCCESS.
 *
 * This function is expected to be executed after an interrupt has been sent
 * to the IA from the CSS. This function returns information about the interrupt
 * which is needed by the IA code to properly handle the interrupt. This
 * information includes the image pipe, buffer type etc.
 */
enum ia_css_err
ia_css_irq_translate(unsigned int *info);

/* @brief Get CSI receiver error info.
 *
 * @param[out] irq_bits	Pointer to the interrupt bits. The interrupt
 *			bits will be written this info.
 *			This will be the error bits that are enabled in the CSI
 *			receiver error register.
 * @return	None
 *
 * This function should be used whenever a CSI receiver error interrupt is
 * generated. It provides the detailed information (bits) on the exact error
 * that occurred.
 *
 *@deprecated {this function is DEPRECATED since it only works on CSI port 1.
 * Use the function below instead and specify the appropriate port.}
 */
void
ia_css_rx_get_irq_info(unsigned int *irq_bits);

/* @brief Get CSI receiver error info.
 *
 * @param[in]  port     Input port identifier.
 * @param[out] irq_bits	Pointer to the interrupt bits. The interrupt
 *			bits will be written this info.
 *			This will be the error bits that are enabled in the CSI
 *			receiver error register.
 * @return	None
 *
 * This function should be used whenever a CSI receiver error interrupt is
 * generated. It provides the detailed information (bits) on the exact error
 * that occurred.
 */
void
ia_css_rx_port_get_irq_info(enum ia_css_csi2_port port, unsigned int *irq_bits);

/* @brief Clear CSI receiver error info.
 *
 * @param[in] irq_bits	The bits that should be cleared from the CSI receiver
 *			interrupt bits register.
 * @return	None
 *
 * This function should be called after ia_css_rx_get_irq_info has been called
 * and the error bits have been interpreted. It is advised to use the return
 * value of that function as the argument to this function to make sure no new
 * error bits get overwritten.
 *
 * @deprecated{this function is DEPRECATED since it only works on CSI port 1.
 * Use the function below instead and specify the appropriate port.}
 */
void
ia_css_rx_clear_irq_info(unsigned int irq_bits);

/* @brief Clear CSI receiver error info.
 *
 * @param[in] port      Input port identifier.
 * @param[in] irq_bits	The bits that should be cleared from the CSI receiver
 *			interrupt bits register.
 * @return	None
 *
 * This function should be called after ia_css_rx_get_irq_info has been called
 * and the error bits have been interpreted. It is advised to use the return
 * value of that function as the argument to this function to make sure no new
 * error bits get overwritten.
 */
void
ia_css_rx_port_clear_irq_info(enum ia_css_csi2_port port, unsigned int irq_bits);

/* @brief Enable or disable specific interrupts.
 *
 * @param[in] type	The interrupt type that will be enabled/disabled.
 * @param[in] enable	enable or disable.
 * @return		Returns IA_CSS_INTERNAL_ERROR if this interrupt
 *			type cannot be enabled/disabled which is true for
 *			CSS internal interrupts. Otherwise returns
 *			IA_CSS_SUCCESS.
 */
enum ia_css_err
ia_css_irq_enable(enum ia_css_irq_info type, bool enable);

#endif /* __IA_CSS_IRQ_H */