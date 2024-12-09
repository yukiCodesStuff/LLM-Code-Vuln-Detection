{
	return ia_css_debug_trace_level;
}

void atomisp_css2_hw_store_8(hrt_address addr, uint8_t data)
{
	unsigned long flags;

	spin_lock_irqsave(&mmio_lock, flags);
	_hrt_master_port_store_8(addr, data);
	spin_unlock_irqrestore(&mmio_lock, flags);
}

static void atomisp_css2_hw_store_16(hrt_address addr, uint16_t data)
{
	unsigned long flags;

	spin_lock_irqsave(&mmio_lock, flags);
	_hrt_master_port_store_16(addr, data);
	spin_unlock_irqrestore(&mmio_lock, flags);
}

static void atomisp_css2_hw_store_32(hrt_address addr, uint32_t data)
{
	unsigned long flags;

	spin_lock_irqsave(&mmio_lock, flags);
	_hrt_master_port_store_32(addr, data);
	spin_unlock_irqrestore(&mmio_lock, flags);
}

static uint8_t atomisp_css2_hw_load_8(hrt_address addr)
{
	unsigned long flags;
	uint8_t ret;

	spin_lock_irqsave(&mmio_lock, flags);
	ret = _hrt_master_port_load_8(addr);
	spin_unlock_irqrestore(&mmio_lock, flags);
	return ret;
}

uint16_t atomisp_css2_hw_load_16(hrt_address addr)
{
	unsigned long flags;
	uint16_t ret;

	spin_lock_irqsave(&mmio_lock, flags);
	ret = _hrt_master_port_load_16(addr);
	spin_unlock_irqrestore(&mmio_lock, flags);
	return ret;
}
uint32_t atomisp_css2_hw_load_32(hrt_address addr)
{
	unsigned long flags;
	uint32_t ret;

	spin_lock_irqsave(&mmio_lock, flags);
	ret = _hrt_master_port_load_32(addr);
	spin_unlock_irqrestore(&mmio_lock, flags);
	return ret;
}

static void atomisp_css2_hw_store(hrt_address addr,
				  const void *from, uint32_t n)
{
	unsigned long flags;
	unsigned int i;
	unsigned int _to = (unsigned int)addr;
	const char *_from = (const char *)from;

	spin_lock_irqsave(&mmio_lock, flags);
	for (i = 0; i < n; i++, _to++, _from++)
		_hrt_master_port_store_8(_to , *_from);
	spin_unlock_irqrestore(&mmio_lock, flags);
}

static void atomisp_css2_hw_load(hrt_address addr, void *to, uint32_t n)
{
	unsigned long flags;
	unsigned int i;
	char *_to = (char *)to;
	unsigned int _from = (unsigned int)addr;

	spin_lock_irqsave(&mmio_lock, flags);
	for (i = 0; i < n; i++, _to++, _from++)
		*_to = _hrt_master_port_load_8(_from);
	spin_unlock_irqrestore(&mmio_lock, flags);
}

static int atomisp_css2_dbg_print(const char *fmt, va_list args)
{
	vprintk(fmt, args);
	return 0;
}

static int atomisp_css2_dbg_ftrace_print(const char *fmt, va_list args)
{
	ftrace_vprintk(fmt, args);
	return 0;
}

static int atomisp_css2_err_print(const char *fmt, va_list args)
{
	vprintk(fmt, args);
	return 0;
}

void atomisp_store_uint32(hrt_address addr, uint32_t data)
{
	atomisp_css2_hw_store_32(addr, data);
}

void atomisp_load_uint32(hrt_address addr, uint32_t *data)
{
	*data = atomisp_css2_hw_load_32(addr);
}
static int hmm_get_mmu_base_addr(unsigned int *mmu_base_addr)
{
	if (sh_mmu_mrfld.get_pd_base == NULL) {
		dev_err(atomisp_dev, "get mmu base address failed.\n");
		return -EINVAL;
	}
{
	unsigned long flags;
	uint8_t ret;

	spin_lock_irqsave(&mmio_lock, flags);
	ret = _hrt_master_port_load_8(addr);
	spin_unlock_irqrestore(&mmio_lock, flags);
	return ret;
}

uint16_t atomisp_css2_hw_load_16(hrt_address addr)
{
	unsigned long flags;
	uint16_t ret;

	spin_lock_irqsave(&mmio_lock, flags);
	ret = _hrt_master_port_load_16(addr);
	spin_unlock_irqrestore(&mmio_lock, flags);
	return ret;
}
uint32_t atomisp_css2_hw_load_32(hrt_address addr)
{
	unsigned long flags;
	uint32_t ret;

	spin_lock_irqsave(&mmio_lock, flags);
	ret = _hrt_master_port_load_32(addr);
	spin_unlock_irqrestore(&mmio_lock, flags);
	return ret;
}

static void atomisp_css2_hw_store(hrt_address addr,
				  const void *from, uint32_t n)
{
	unsigned long flags;
	unsigned int i;
	unsigned int _to = (unsigned int)addr;
	const char *_from = (const char *)from;

	spin_lock_irqsave(&mmio_lock, flags);
	for (i = 0; i < n; i++, _to++, _from++)
		_hrt_master_port_store_8(_to , *_from);
	spin_unlock_irqrestore(&mmio_lock, flags);
}

static void atomisp_css2_hw_load(hrt_address addr, void *to, uint32_t n)
{
	unsigned long flags;
	unsigned int i;
	char *_to = (char *)to;
	unsigned int _from = (unsigned int)addr;

	spin_lock_irqsave(&mmio_lock, flags);
	for (i = 0; i < n; i++, _to++, _from++)
		*_to = _hrt_master_port_load_8(_from);
	spin_unlock_irqrestore(&mmio_lock, flags);
}

static int atomisp_css2_dbg_print(const char *fmt, va_list args)
{
	vprintk(fmt, args);
	return 0;
}

static int atomisp_css2_dbg_ftrace_print(const char *fmt, va_list args)
{
	ftrace_vprintk(fmt, args);
	return 0;
}

static int atomisp_css2_err_print(const char *fmt, va_list args)
{
	vprintk(fmt, args);
	return 0;
}

void atomisp_store_uint32(hrt_address addr, uint32_t data)
{
	atomisp_css2_hw_store_32(addr, data);
}

void atomisp_load_uint32(hrt_address addr, uint32_t *data)
{
	*data = atomisp_css2_hw_load_32(addr);
}
static int hmm_get_mmu_base_addr(unsigned int *mmu_base_addr)
{
	if (sh_mmu_mrfld.get_pd_base == NULL) {
		dev_err(atomisp_dev, "get mmu base address failed.\n");
		return -EINVAL;
	}
{
	unsigned long flags;
	uint16_t ret;

	spin_lock_irqsave(&mmio_lock, flags);
	ret = _hrt_master_port_load_16(addr);
	spin_unlock_irqrestore(&mmio_lock, flags);
	return ret;
}
uint32_t atomisp_css2_hw_load_32(hrt_address addr)
{
	unsigned long flags;
	uint32_t ret;

	spin_lock_irqsave(&mmio_lock, flags);
	ret = _hrt_master_port_load_32(addr);
	spin_unlock_irqrestore(&mmio_lock, flags);
	return ret;
}

static void atomisp_css2_hw_store(hrt_address addr,
				  const void *from, uint32_t n)
{
	unsigned long flags;
	unsigned int i;
	unsigned int _to = (unsigned int)addr;
	const char *_from = (const char *)from;

	spin_lock_irqsave(&mmio_lock, flags);
	for (i = 0; i < n; i++, _to++, _from++)
		_hrt_master_port_store_8(_to , *_from);
	spin_unlock_irqrestore(&mmio_lock, flags);
}

static void atomisp_css2_hw_load(hrt_address addr, void *to, uint32_t n)
{
	unsigned long flags;
	unsigned int i;
	char *_to = (char *)to;
	unsigned int _from = (unsigned int)addr;

	spin_lock_irqsave(&mmio_lock, flags);
	for (i = 0; i < n; i++, _to++, _from++)
		*_to = _hrt_master_port_load_8(_from);
	spin_unlock_irqrestore(&mmio_lock, flags);
}

static int atomisp_css2_dbg_print(const char *fmt, va_list args)
{
	vprintk(fmt, args);
	return 0;
}

static int atomisp_css2_dbg_ftrace_print(const char *fmt, va_list args)
{
	ftrace_vprintk(fmt, args);
	return 0;
}

static int atomisp_css2_err_print(const char *fmt, va_list args)
{
	vprintk(fmt, args);
	return 0;
}

void atomisp_store_uint32(hrt_address addr, uint32_t data)
{
	atomisp_css2_hw_store_32(addr, data);
}

void atomisp_load_uint32(hrt_address addr, uint32_t *data)
{
	*data = atomisp_css2_hw_load_32(addr);
}
static int hmm_get_mmu_base_addr(unsigned int *mmu_base_addr)
{
	if (sh_mmu_mrfld.get_pd_base == NULL) {
		dev_err(atomisp_dev, "get mmu base address failed.\n");
		return -EINVAL;
	}
	if (err != IA_CSS_SUCCESS) {
		dev_warn(isp->dev,
			  "%s:failed to translate irq (err = %d,infos = %d)\n",
			  __func__, err, *infos);
		return -EINVAL;
	}

	return 0;
}

void atomisp_css_rx_get_irq_info(enum ia_css_csi2_port port,
					unsigned int *infos)
{
#ifndef ISP2401_NEW_INPUT_SYSTEM
	ia_css_isys_rx_get_irq_info(port, infos);
#else
	*infos = 0;
#endif
}

void atomisp_css_rx_clear_irq_info(enum ia_css_csi2_port port,
					unsigned int infos)
{
#ifndef ISP2401_NEW_INPUT_SYSTEM
	ia_css_isys_rx_clear_irq_info(port, infos);
#endif
}

int atomisp_css_irq_enable(struct atomisp_device *isp,
			    enum atomisp_css_irq_info info, bool enable)
{
	if (ia_css_irq_enable(info, enable) != IA_CSS_SUCCESS) {
		dev_warn(isp->dev, "%s:Invalid irq info.\n", __func__);
		return -EINVAL;
	}
{
#ifndef ISP2401_NEW_INPUT_SYSTEM
	ia_css_isys_rx_get_irq_info(port, infos);
#else
	*infos = 0;
#endif
}

void atomisp_css_rx_clear_irq_info(enum ia_css_csi2_port port,
					unsigned int infos)
{
#ifndef ISP2401_NEW_INPUT_SYSTEM
	ia_css_isys_rx_clear_irq_info(port, infos);
#endif
}

int atomisp_css_irq_enable(struct atomisp_device *isp,
			    enum atomisp_css_irq_info info, bool enable)
{
	if (ia_css_irq_enable(info, enable) != IA_CSS_SUCCESS) {
		dev_warn(isp->dev, "%s:Invalid irq info.\n", __func__);
		return -EINVAL;
	}
{
	ia_css_mmu_invalidate_cache();
}

void atomisp_css_mmu_set_page_table_base_index(unsigned long base_index)
{
}

/*
 * Check whether currently running MIPI buffer size fulfill
 * the requirement of the stream to be run
 */
bool __need_realloc_mipi_buffer(struct atomisp_device *isp)
{
	unsigned int i;

	for (i = 0; i < isp->num_of_streams; i++) {
		struct atomisp_sub_device *asd = &isp->asd[i];

		if (asd->streaming !=
				ATOMISP_DEVICE_STREAMING_ENABLED)
			continue;
		if (asd->mipi_frame_size < isp->mipi_frame_size)
			return true;
	}
{
	struct ia_css_stream_config *s_config =
		&asd->stream_env[stream_id].stream_config;

	s_config->isys_config[isys_stream].valid = valid;
}

void atomisp_css_isys_set_format(struct atomisp_sub_device *asd,
				 enum atomisp_input_stream_id stream_id,
				 enum atomisp_css_stream_format format,
				 int isys_stream)
{

	struct ia_css_stream_config *s_config =
			&asd->stream_env[stream_id].stream_config;

	s_config->isys_config[isys_stream].format = format;
}
{

	struct ia_css_stream_config *s_config =
			&asd->stream_env[stream_id].stream_config;

	s_config->isys_config[isys_stream].format = format;
}

void atomisp_css_input_set_format(struct atomisp_sub_device *asd,
					enum atomisp_input_stream_id stream_id,
					enum atomisp_css_stream_format format)
{

	struct ia_css_stream_config *s_config =
			&asd->stream_env[stream_id].stream_config;

	s_config->input_config.format = format;
}
{
	int i;
	struct ia_css_stream_config *s_config =
			&asd->stream_env[stream_id].stream_config;
	/*
	 * Set all isys configs to not valid.
	 * Currently we support only one stream per channel
	 */
	for (i = IA_CSS_STREAM_ISYS_STREAM_0;
	     i < IA_CSS_STREAM_MAX_ISYS_STREAM_PER_CH; i++)
		s_config->isys_config[i].valid = false;

	atomisp_css_isys_set_resolution(asd, stream_id, ffmt,
					IA_CSS_STREAM_DEFAULT_ISYS_STREAM_IDX);
	atomisp_css_isys_set_format(asd, stream_id,
				    s_config->input_config.format,
				    IA_CSS_STREAM_DEFAULT_ISYS_STREAM_IDX);
	atomisp_css_isys_set_link(asd, stream_id, NO_LINK,
				  IA_CSS_STREAM_DEFAULT_ISYS_STREAM_IDX);
	atomisp_css_isys_set_valid(asd, stream_id, true,
				   IA_CSS_STREAM_DEFAULT_ISYS_STREAM_IDX);

	return 0;
}

int atomisp_css_isys_two_stream_cfg(struct atomisp_sub_device *asd,
				    enum atomisp_input_stream_id stream_id,
				    enum atomisp_css_stream_format input_format)
{
	struct ia_css_stream_config *s_config =
		&asd->stream_env[stream_id].stream_config;

	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].input_res.width =
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_0].input_res.width;

	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].input_res.height =
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_0].input_res.height / 2;

	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].linked_isys_stream_id
		= IA_CSS_STREAM_ISYS_STREAM_0;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_0].format =
		IA_CSS_STREAM_FORMAT_USER_DEF1;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].format =
		IA_CSS_STREAM_FORMAT_USER_DEF2;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].valid = true;
	return 0;
}
{
	struct ia_css_stream_config *s_config =
		&asd->stream_env[stream_id].stream_config;

	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].input_res.width =
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_0].input_res.width;

	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].input_res.height =
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_0].input_res.height / 2;

	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].linked_isys_stream_id
		= IA_CSS_STREAM_ISYS_STREAM_0;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_0].format =
		IA_CSS_STREAM_FORMAT_USER_DEF1;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].format =
		IA_CSS_STREAM_FORMAT_USER_DEF2;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].valid = true;
	return 0;
}

void atomisp_css_isys_two_stream_cfg_update_stream1(
				    struct atomisp_sub_device *asd,
				    enum atomisp_input_stream_id stream_id,
				    enum atomisp_css_stream_format input_format,
				    unsigned int width, unsigned int height)
{
	struct ia_css_stream_config *s_config =
		&asd->stream_env[stream_id].stream_config;

	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_0].input_res.width =
		width;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_0].input_res.height =
		height;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_0].format =
		input_format;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_0].valid = true;
}

void atomisp_css_isys_two_stream_cfg_update_stream2(
				    struct atomisp_sub_device *asd,
				    enum atomisp_input_stream_id stream_id,
				    enum atomisp_css_stream_format input_format,
				    unsigned int width, unsigned int height)
{
	struct ia_css_stream_config *s_config =
		&asd->stream_env[stream_id].stream_config;

	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].input_res.width =
		width;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].input_res.height =
	height;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].linked_isys_stream_id
		= IA_CSS_STREAM_ISYS_STREAM_0;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].format =
		input_format;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].valid = true;
}

int atomisp_css_input_set_effective_resolution(
					struct atomisp_sub_device *asd,
					enum atomisp_input_stream_id stream_id,
					unsigned int width, unsigned int height)
{
	struct ia_css_stream_config *s_config =
			&asd->stream_env[stream_id].stream_config;
	s_config->input_config.effective_res.width = width;
	s_config->input_config.effective_res.height = height;
	return 0;
}

void atomisp_css_video_set_dis_envelope(struct atomisp_sub_device *asd,
					unsigned int dvs_w, unsigned int dvs_h)
{
	asd->stream_env[ATOMISP_INPUT_STREAM_GENERAL]
		.pipe_configs[IA_CSS_PIPE_ID_VIDEO].dvs_envelope.width = dvs_w;
	asd->stream_env[ATOMISP_INPUT_STREAM_GENERAL]
		.pipe_configs[IA_CSS_PIPE_ID_VIDEO].dvs_envelope.height = dvs_h;
}

void atomisp_css_input_set_two_pixels_per_clock(
					struct atomisp_sub_device *asd,
					bool two_ppc)
{
	int i;

	if (asd->stream_env[ATOMISP_INPUT_STREAM_GENERAL]
		.stream_config.pixels_per_clock == (two_ppc ? 2 : 1))
		return;

	asd->stream_env[ATOMISP_INPUT_STREAM_GENERAL]
		.stream_config.pixels_per_clock = (two_ppc ? 2 : 1);
	for (i = 0; i < IA_CSS_PIPE_ID_NUM; i++)
		asd->stream_env[ATOMISP_INPUT_STREAM_GENERAL]
		.update_pipe[i] = true;
}

void atomisp_css_enable_raw_binning(struct atomisp_sub_device *asd,
					bool enable)
{
	struct atomisp_stream_env *stream_env =
		&asd->stream_env[ATOMISP_INPUT_STREAM_GENERAL];
	unsigned int pipe;

	if (asd->run_mode->val == ATOMISP_RUN_MODE_VIDEO)
		pipe = IA_CSS_PIPE_ID_VIDEO;
	else
		pipe = IA_CSS_PIPE_ID_PREVIEW;

	stream_env->pipe_extra_configs[pipe].enable_raw_binning = enable;
	stream_env->update_pipe[pipe] = true;
	if (enable)
		stream_env->pipe_configs[pipe].output_info[0].padded_width =
			stream_env->stream_config.input_config.effective_res.width;
}

void atomisp_css_enable_dz(struct atomisp_sub_device *asd, bool enable)
{
	int i;

	for (i = 0; i < IA_CSS_PIPE_ID_NUM; i++)
		asd->stream_env[ATOMISP_INPUT_STREAM_GENERAL]
			.pipe_configs[i].enable_dz = enable;
}

void atomisp_css_capture_set_mode(struct atomisp_sub_device *asd,
				enum atomisp_css_capture_mode mode)
{
	struct atomisp_stream_env *stream_env =
		&asd->stream_env[ATOMISP_INPUT_STREAM_GENERAL];

	if (stream_env->pipe_configs[IA_CSS_PIPE_ID_CAPTURE]
		.default_capture_config.mode == mode)
		return;

	stream_env->pipe_configs[IA_CSS_PIPE_ID_CAPTURE].
					default_capture_config.mode = mode;
	stream_env->update_pipe[IA_CSS_PIPE_ID_CAPTURE] = true;
}

void atomisp_css_input_set_mode(struct atomisp_sub_device *asd,
				enum atomisp_css_input_mode mode)
{
	int i;
	struct atomisp_device *isp = asd->isp;
	unsigned int size_mem_words;

	for (i = 0; i < ATOMISP_INPUT_STREAM_NUM; i++)
		asd->stream_env[i].stream_config.mode = mode;

	if (isp->inputs[asd->input_curr].type == TEST_PATTERN) {
		struct ia_css_stream_config *s_config =
		    &asd->stream_env[ATOMISP_INPUT_STREAM_GENERAL].stream_config;
		s_config->mode = IA_CSS_INPUT_MODE_TPG;
		s_config->source.tpg.mode = IA_CSS_TPG_MODE_CHECKERBOARD;
		s_config->source.tpg.x_mask = (1 << 4) - 1;
		s_config->source.tpg.x_delta = -2;
		s_config->source.tpg.y_mask = (1 << 4) - 1;
		s_config->source.tpg.y_delta = 3;
		s_config->source.tpg.xy_mask = (1 << 8) - 1;
		return;
	}
{
	struct ia_css_stream_config *s_config =
		&asd->stream_env[stream_id].stream_config;

	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_0].input_res.width =
		width;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_0].input_res.height =
		height;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_0].format =
		input_format;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_0].valid = true;
}

void atomisp_css_isys_two_stream_cfg_update_stream2(
				    struct atomisp_sub_device *asd,
				    enum atomisp_input_stream_id stream_id,
				    enum atomisp_css_stream_format input_format,
				    unsigned int width, unsigned int height)
{
	struct ia_css_stream_config *s_config =
		&asd->stream_env[stream_id].stream_config;

	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].input_res.width =
		width;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].input_res.height =
	height;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].linked_isys_stream_id
		= IA_CSS_STREAM_ISYS_STREAM_0;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].format =
		input_format;
	s_config->isys_config[IA_CSS_STREAM_ISYS_STREAM_1].valid = true;
}
	if (stream_env->stream_config.disable_cont_viewfinder != !enable) {
		stream_env->stream_config.disable_cont_viewfinder = !enable;
		for (i = 0; i < IA_CSS_PIPE_ID_NUM; i++)
			stream_env->update_pipe[i] = true;
	}
}

int atomisp_css_input_configure_port(
		struct atomisp_sub_device *asd,
		mipi_port_ID_t port,
		unsigned int num_lanes,
		unsigned int timeout,
		unsigned int mipi_freq,
		enum atomisp_css_stream_format metadata_format,
		unsigned int metadata_width,
		unsigned int metadata_height)
{
	int i;
	struct atomisp_stream_env *stream_env;
	/*
	 * Calculate rx_count as follows:
	 * Input: mipi_freq                 : CSI-2 bus frequency in Hz
	 * UI = 1 / (2 * mipi_freq)         : period of one bit on the bus
	 * min = 85e-9 + 6 * UI             : Limits for rx_count in seconds
	 * max = 145e-9 + 10 * UI
	 * rxcount0 = min / (4 / mipi_freq) : convert seconds to byte clocks
	 * rxcount = rxcount0 - 2           : adjust for better results
	 * The formula below is simplified version of the above with
	 * 10-bit fixed points for improved accuracy.
	 */
	const unsigned int rxcount =
		min(((mipi_freq / 46000) - 1280) >> 10, 0xffU) * 0x01010101U;

	for (i = 0; i < ATOMISP_INPUT_STREAM_NUM; i++) {
		stream_env = &asd->stream_env[i];
		stream_env->stream_config.source.port.port = port;
		stream_env->stream_config.source.port.num_lanes = num_lanes;
		stream_env->stream_config.source.port.timeout = timeout;
		if (mipi_freq)
			stream_env->stream_config.source.port.rxcount = rxcount;
		stream_env->stream_config.
			metadata_config.data_type = metadata_format;
		stream_env->stream_config.
			metadata_config.resolution.width = metadata_width;
		stream_env->stream_config.
			metadata_config.resolution.height = metadata_height;
	}

	return 0;
}
		switch (type) {
		case ATOMISP_CSS_VF_FRAME:
			*info = p_info.vf_output_info[0];
			dev_dbg(isp->dev, "getting vf frame info.\n");
			break;
		case ATOMISP_CSS_SECOND_VF_FRAME:
			*info = p_info.vf_output_info[1];
			dev_dbg(isp->dev, "getting second vf frame info.\n");
			break;
		case ATOMISP_CSS_OUTPUT_FRAME:
			*info = p_info.output_info[0];
			dev_dbg(isp->dev, "getting main frame info.\n");
			break;
		case ATOMISP_CSS_SECOND_OUTPUT_FRAME:
			*info = p_info.output_info[1];
			dev_dbg(isp->dev, "getting second main frame info.\n");
			break;
		case ATOMISP_CSS_RAW_FRAME:
			*info = p_info.raw_output_info;
			dev_dbg(isp->dev, "getting raw frame info.\n");
		}
		dev_dbg(isp->dev, "get frame info: w=%d, h=%d, num_invalid_frames %d.\n",
			info->res.width, info->res.height, p_info.num_invalid_frames);
		return 0;
	}

stream_err:
	__destroy_pipes(asd, true);
	return -EINVAL;
}

unsigned int atomisp_get_pipe_index(struct atomisp_sub_device *asd,
					uint16_t source_pad)
{
	struct atomisp_device *isp = asd->isp;
	/*
	 * to SOC camera, use yuvpp pipe.
	 */
	if (ATOMISP_USE_YUVPP(asd))
		return IA_CSS_PIPE_ID_YUVPP;

	switch (source_pad) {
	case ATOMISP_SUBDEV_PAD_SOURCE_VIDEO:
		if (asd->yuvpp_mode)
			return IA_CSS_PIPE_ID_YUVPP;
		if (asd->copy_mode)
			return IA_CSS_PIPE_ID_COPY;
		if (asd->run_mode->val == ATOMISP_RUN_MODE_VIDEO
		    || asd->vfpp->val == ATOMISP_VFPP_DISABLE_SCALER)
			return IA_CSS_PIPE_ID_VIDEO;
		else
			return IA_CSS_PIPE_ID_CAPTURE;
	case ATOMISP_SUBDEV_PAD_SOURCE_CAPTURE:
		if (asd->copy_mode)
			return IA_CSS_PIPE_ID_COPY;
		return IA_CSS_PIPE_ID_CAPTURE;
	case ATOMISP_SUBDEV_PAD_SOURCE_VF:
		if (!atomisp_is_mbuscode_raw(
		    asd->fmt[asd->capture_pad].fmt.code))
			return IA_CSS_PIPE_ID_CAPTURE;
	case ATOMISP_SUBDEV_PAD_SOURCE_PREVIEW:
		if (asd->yuvpp_mode)
			return IA_CSS_PIPE_ID_YUVPP;
		if (asd->copy_mode)
			return IA_CSS_PIPE_ID_COPY;
		if (asd->run_mode->val == ATOMISP_RUN_MODE_VIDEO)
			return IA_CSS_PIPE_ID_VIDEO;
		else
			return IA_CSS_PIPE_ID_PREVIEW;
	}