#include "sh_css_internal.h"

#if !defined(USE_INPUT_SYSTEM_VERSION_2401)
void ia_css_isys_rx_enable_all_interrupts(mipi_port_ID_t port)
{
	hrt_data bits = receiver_port_reg_load(RX0_ID,
				port,
				_HRT_CSS_RECEIVER_IRQ_ENABLE_REG_IDX);
 * initializers in Windows. Without that there is no easy way to guarantee
 * that the array values would be in the correct order.
 * */
mipi_port_ID_t ia_css_isys_port_to_mipi_port(enum ia_css_csi2_port api_port)
{
	/* In this module the validity of the inptu variable should
	 * have been checked already, so we do not check for erroneous
	 * values. */
	mipi_port_ID_t port = MIPI_PORT0_ID;

	if (api_port == IA_CSS_CSI2_PORT1)
		port = MIPI_PORT1_ID;
	else if (api_port == IA_CSS_CSI2_PORT2)
		port = MIPI_PORT2_ID;

	return port;
}

unsigned int ia_css_isys_rx_get_interrupt_reg(mipi_port_ID_t port)
{
	return receiver_port_reg_load(RX0_ID,
				      port,
				      _HRT_CSS_RECEIVER_IRQ_STATUS_REG_IDX);

void ia_css_rx_get_irq_info(unsigned int *irq_infos)
{
	ia_css_rx_port_get_irq_info(IA_CSS_CSI2_PORT1, irq_infos);
}

void ia_css_rx_port_get_irq_info(enum ia_css_csi2_port api_port,
				 unsigned int *irq_infos)
{
	mipi_port_ID_t port = ia_css_isys_port_to_mipi_port(api_port);
	ia_css_isys_rx_get_irq_info(port, irq_infos);
}

void ia_css_isys_rx_get_irq_info(mipi_port_ID_t port,
				 unsigned int *irq_infos)
{
	unsigned int bits;


void ia_css_rx_clear_irq_info(unsigned int irq_infos)
{
	ia_css_rx_port_clear_irq_info(IA_CSS_CSI2_PORT1, irq_infos);
}

void ia_css_rx_port_clear_irq_info(enum ia_css_csi2_port api_port, unsigned int irq_infos)
{
	mipi_port_ID_t port = ia_css_isys_port_to_mipi_port(api_port);
	ia_css_isys_rx_clear_irq_info(port, irq_infos);
}

void ia_css_isys_rx_clear_irq_info(mipi_port_ID_t port, unsigned int irq_infos)
{
	hrt_data bits = receiver_port_reg_load(RX0_ID,
				port,
				_HRT_CSS_RECEIVER_IRQ_ENABLE_REG_IDX);
#endif /* #if !defined(USE_INPUT_SYSTEM_VERSION_2401) */

enum ia_css_err ia_css_isys_convert_stream_format_to_mipi_format(
		enum ia_css_stream_format input_format,
		mipi_predictor_t compression,
		unsigned int *fmt_type)
{
	assert(fmt_type != NULL);
	 */
	if (compression != MIPI_PREDICTOR_NONE) {
		switch (input_format) {
		case IA_CSS_STREAM_FORMAT_RAW_6:
			*fmt_type = 6;
			break;
		case IA_CSS_STREAM_FORMAT_RAW_7:
			*fmt_type = 7;
			break;
		case IA_CSS_STREAM_FORMAT_RAW_8:
			*fmt_type = 8;
			break;
		case IA_CSS_STREAM_FORMAT_RAW_10:
			*fmt_type = 10;
			break;
		case IA_CSS_STREAM_FORMAT_RAW_12:
			*fmt_type = 12;
			break;
		case IA_CSS_STREAM_FORMAT_RAW_14:
			*fmt_type = 14;
			break;
		case IA_CSS_STREAM_FORMAT_RAW_16:
			*fmt_type = 16;
			break;
		default:
			return IA_CSS_ERR_INTERNAL_ERROR;
	 * MW: For some reason the mapping is not 1-to-1
	 */
	switch (input_format) {
	case IA_CSS_STREAM_FORMAT_RGB_888:
		*fmt_type = MIPI_FORMAT_RGB888;
		break;
	case IA_CSS_STREAM_FORMAT_RGB_555:
		*fmt_type = MIPI_FORMAT_RGB555;
		break;
	case IA_CSS_STREAM_FORMAT_RGB_444:
		*fmt_type = MIPI_FORMAT_RGB444;
		break;
	case IA_CSS_STREAM_FORMAT_RGB_565:
		*fmt_type = MIPI_FORMAT_RGB565;
		break;
	case IA_CSS_STREAM_FORMAT_RGB_666:
		*fmt_type = MIPI_FORMAT_RGB666;
		break;
	case IA_CSS_STREAM_FORMAT_RAW_8:
		*fmt_type = MIPI_FORMAT_RAW8;
		break;
	case IA_CSS_STREAM_FORMAT_RAW_10:
		*fmt_type = MIPI_FORMAT_RAW10;
		break;
	case IA_CSS_STREAM_FORMAT_RAW_6:
		*fmt_type = MIPI_FORMAT_RAW6;
		break;
	case IA_CSS_STREAM_FORMAT_RAW_7:
		*fmt_type = MIPI_FORMAT_RAW7;
		break;
	case IA_CSS_STREAM_FORMAT_RAW_12:
		*fmt_type = MIPI_FORMAT_RAW12;
		break;
	case IA_CSS_STREAM_FORMAT_RAW_14:
		*fmt_type = MIPI_FORMAT_RAW14;
		break;
	case IA_CSS_STREAM_FORMAT_YUV420_8:
		*fmt_type = MIPI_FORMAT_YUV420_8;
		break;
	case IA_CSS_STREAM_FORMAT_YUV420_10:
		*fmt_type = MIPI_FORMAT_YUV420_10;
		break;
	case IA_CSS_STREAM_FORMAT_YUV422_8:
		*fmt_type = MIPI_FORMAT_YUV422_8;
		break;
	case IA_CSS_STREAM_FORMAT_YUV422_10:
		*fmt_type = MIPI_FORMAT_YUV422_10;
		break;
	case IA_CSS_STREAM_FORMAT_YUV420_8_LEGACY:
		*fmt_type = MIPI_FORMAT_YUV420_8_LEGACY;
		break;
	case IA_CSS_STREAM_FORMAT_EMBEDDED:
		*fmt_type = MIPI_FORMAT_EMBEDDED;
		break;
#ifndef USE_INPUT_SYSTEM_VERSION_2401
	case IA_CSS_STREAM_FORMAT_RAW_16:
		/* This is not specified by Arasan, so we use
		 * 17 for now.
		 */
		*fmt_type = MIPI_FORMAT_RAW16;
		break;
	case IA_CSS_STREAM_FORMAT_BINARY_8:
		*fmt_type = MIPI_FORMAT_BINARY_8;
		break;
#else
	case IA_CSS_STREAM_FORMAT_USER_DEF1:
		*fmt_type = MIPI_FORMAT_CUSTOM0;
		break;
	case IA_CSS_STREAM_FORMAT_USER_DEF2:
		*fmt_type = MIPI_FORMAT_CUSTOM1;
		break;
	case IA_CSS_STREAM_FORMAT_USER_DEF3:
		*fmt_type = MIPI_FORMAT_CUSTOM2;
		break;
	case IA_CSS_STREAM_FORMAT_USER_DEF4:
		*fmt_type = MIPI_FORMAT_CUSTOM3;
		break;
	case IA_CSS_STREAM_FORMAT_USER_DEF5:
		*fmt_type = MIPI_FORMAT_CUSTOM4;
		break;
	case IA_CSS_STREAM_FORMAT_USER_DEF6:
		*fmt_type = MIPI_FORMAT_CUSTOM5;
		break;
	case IA_CSS_STREAM_FORMAT_USER_DEF7:
		*fmt_type = MIPI_FORMAT_CUSTOM6;
		break;
	case IA_CSS_STREAM_FORMAT_USER_DEF8:
		*fmt_type = MIPI_FORMAT_CUSTOM7;
		break;
#endif

	case IA_CSS_STREAM_FORMAT_YUV420_16:
	case IA_CSS_STREAM_FORMAT_YUV422_16:
	default:
		return IA_CSS_ERR_INTERNAL_ERROR;
	}
	return IA_CSS_SUCCESS;
}

unsigned int ia_css_csi2_calculate_input_system_alignment(
	enum ia_css_stream_format fmt_type)
{
	unsigned int memory_alignment_in_bytes = HIVE_ISP_DDR_WORD_BYTES;

	switch (fmt_type) {
	case IA_CSS_STREAM_FORMAT_RAW_6:
	case IA_CSS_STREAM_FORMAT_RAW_7:
	case IA_CSS_STREAM_FORMAT_RAW_8:
	case IA_CSS_STREAM_FORMAT_RAW_10:
	case IA_CSS_STREAM_FORMAT_RAW_12:
	case IA_CSS_STREAM_FORMAT_RAW_14:
		memory_alignment_in_bytes = 2 * ISP_VEC_NELEMS;
		break;
	case IA_CSS_STREAM_FORMAT_YUV420_8:
	case IA_CSS_STREAM_FORMAT_YUV422_8:
	case IA_CSS_STREAM_FORMAT_USER_DEF1:
	case IA_CSS_STREAM_FORMAT_USER_DEF2:
	case IA_CSS_STREAM_FORMAT_USER_DEF3:
	case IA_CSS_STREAM_FORMAT_USER_DEF4:
	case IA_CSS_STREAM_FORMAT_USER_DEF5:
	case IA_CSS_STREAM_FORMAT_USER_DEF6:
	case IA_CSS_STREAM_FORMAT_USER_DEF7:
	case IA_CSS_STREAM_FORMAT_USER_DEF8:
		/* Planar YUV formats need to have all planes aligned, this means
		 * double the alignment for the Y plane if the horizontal decimation is 2. */
		memory_alignment_in_bytes = 2 * HIVE_ISP_DDR_WORD_BYTES;
		break;
	case IA_CSS_STREAM_FORMAT_EMBEDDED:
	default:
		memory_alignment_in_bytes = HIVE_ISP_DDR_WORD_BYTES;
		break;
	}
#if defined(HAS_RX_VERSION_2)
	bool port_enabled[N_MIPI_PORT_ID];
	bool any_port_enabled = false;
	mipi_port_ID_t port;

	if ((config == NULL)
		|| (config->mode >= N_RX_MODE)
		|| (config->port >= N_MIPI_PORT_ID)) {
		assert(0);
		return;
	}
	for (port = (mipi_port_ID_t) 0; port < N_MIPI_PORT_ID; port++) {
		if (is_receiver_port_enabled(RX0_ID, port))
			any_port_enabled = true;
	}
	/* AM: Check whether this is a problem with multiple

void ia_css_isys_rx_disable(void)
{
	mipi_port_ID_t port;
	for (port = (mipi_port_ID_t) 0; port < N_MIPI_PORT_ID; port++) {
		receiver_port_reg_store(RX0_ID, port,
					_HRT_CSS_RECEIVER_DEVICE_READY_REG_IDX,
					false);
	}