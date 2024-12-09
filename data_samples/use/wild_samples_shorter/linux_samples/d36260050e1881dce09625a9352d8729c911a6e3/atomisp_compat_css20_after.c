	return ia_css_debug_trace_level;
}

static void atomisp_css2_hw_store_8(hrt_address addr, uint8_t data)
{
	unsigned long flags;

	spin_lock_irqsave(&mmio_lock, flags);
	return ret;
}

static uint16_t atomisp_css2_hw_load_16(hrt_address addr)
{
	unsigned long flags;
	uint16_t ret;

	spin_unlock_irqrestore(&mmio_lock, flags);
	return ret;
}

static uint32_t atomisp_css2_hw_load_32(hrt_address addr)
{
	unsigned long flags;
	uint32_t ret;

	return 0;
}

void atomisp_css_rx_get_irq_info(enum mipi_port_id port,
					unsigned int *infos)
{
#ifndef ISP2401_NEW_INPUT_SYSTEM
	ia_css_isys_rx_get_irq_info(port, infos);
#endif
}

void atomisp_css_rx_clear_irq_info(enum mipi_port_id port,
					unsigned int infos)
{
#ifndef ISP2401_NEW_INPUT_SYSTEM
	ia_css_isys_rx_clear_irq_info(port, infos);
	ia_css_mmu_invalidate_cache();
}

int atomisp_css_start(struct atomisp_sub_device *asd,
			enum atomisp_css_pipe_id pipe_id, bool in_reset)
{
	struct atomisp_device *isp = asd->isp;

void atomisp_css_isys_set_format(struct atomisp_sub_device *asd,
				 enum atomisp_input_stream_id stream_id,
				 enum atomisp_input_format format,
				 int isys_stream)
{

	struct ia_css_stream_config *s_config =

void atomisp_css_input_set_format(struct atomisp_sub_device *asd,
					enum atomisp_input_stream_id stream_id,
					enum atomisp_input_format format)
{

	struct ia_css_stream_config *s_config =
			&asd->stream_env[stream_id].stream_config;

int atomisp_css_isys_two_stream_cfg(struct atomisp_sub_device *asd,
				    enum atomisp_input_stream_id stream_id,
				    enum atomisp_input_format input_format)
{
	struct ia_css_stream_config *s_config =
		&asd->stream_env[stream_id].stream_config;

	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].linked_isys_stream_id
		= IA_CSS_STREAM_ISYS_STREAM_0;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_0].format =
		ATOMISP_INPUT_FORMAT_USER_DEF1;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].format =
		ATOMISP_INPUT_FORMAT_USER_DEF2;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].valid = true;
	return 0;
}

void atomisp_css_isys_two_stream_cfg_update_stream1(
				    struct atomisp_sub_device *asd,
				    enum atomisp_input_stream_id stream_id,
				    enum atomisp_input_format input_format,
				    unsigned int width, unsigned int height)
{
	struct ia_css_stream_config *s_config =
		&asd->stream_env[stream_id].stream_config;
void atomisp_css_isys_two_stream_cfg_update_stream2(
				    struct atomisp_sub_device *asd,
				    enum atomisp_input_stream_id stream_id,
				    enum atomisp_input_format input_format,
				    unsigned int width, unsigned int height)
{
	struct ia_css_stream_config *s_config =
		&asd->stream_env[stream_id].stream_config;

int atomisp_css_input_configure_port(
		struct atomisp_sub_device *asd,
		enum mipi_port_id port,
		unsigned int num_lanes,
		unsigned int timeout,
		unsigned int mipi_freq,
		enum atomisp_input_format metadata_format,
		unsigned int metadata_width,
		unsigned int metadata_height)
{
	int i;
	return -EINVAL;
}

static unsigned int atomisp_get_pipe_index(struct atomisp_sub_device *asd,
					   uint16_t source_pad)
{
	struct atomisp_device *isp = asd->isp;
	/*
	 * to SOC camera, use yuvpp pipe.