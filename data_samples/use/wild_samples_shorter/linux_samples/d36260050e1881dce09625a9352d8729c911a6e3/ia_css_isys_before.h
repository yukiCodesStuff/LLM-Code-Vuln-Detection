#if defined(USE_INPUT_SYSTEM_VERSION_2) || defined(USE_INPUT_SYSTEM_VERSION_2401)
input_system_error_t ia_css_isys_init(void);
void ia_css_isys_uninit(void);
mipi_port_ID_t ia_css_isys_port_to_mipi_port(
	enum ia_css_csi2_port api_port);
#endif

#if defined(USE_INPUT_SYSTEM_VERSION_2401)

 *				there is already a stream registered with the same handle
 */
enum ia_css_err ia_css_isys_csi_rx_register_stream(
	enum ia_css_csi2_port port,
	uint32_t isys_stream_id);

/**
 * @brief Unregister one (virtual) stream. This is used to track when all
 *				there is no stream registered with that handle
 */
enum ia_css_err ia_css_isys_csi_rx_unregister_stream(
	enum ia_css_csi2_port port,
	uint32_t isys_stream_id);

enum ia_css_err ia_css_isys_convert_compressed_format(
		struct ia_css_csi2_compression *comp,
		struct input_system_cfg_s *cfg);
unsigned int ia_css_csi2_calculate_input_system_alignment(
	enum ia_css_stream_format fmt_type);
#endif

#if !defined(USE_INPUT_SYSTEM_VERSION_2401)
/* CSS Receiver */

void ia_css_isys_rx_disable(void);

void ia_css_isys_rx_enable_all_interrupts(mipi_port_ID_t port);

unsigned int ia_css_isys_rx_get_interrupt_reg(mipi_port_ID_t port);
void ia_css_isys_rx_get_irq_info(mipi_port_ID_t port,
				 unsigned int *irq_infos);
void ia_css_isys_rx_clear_irq_info(mipi_port_ID_t port,
				   unsigned int irq_infos);
unsigned int ia_css_isys_rx_translate_irq_infos(unsigned int bits);

#endif /* #if !defined(USE_INPUT_SYSTEM_VERSION_2401) */
 * format type must be sumitted correctly by the application.
 */
enum ia_css_err ia_css_isys_convert_stream_format_to_mipi_format(
		enum ia_css_stream_format input_format,
		mipi_predictor_t compression,
		unsigned int *fmt_type);

#ifdef USE_INPUT_SYSTEM_VERSION_2401