int atomisp_css_irq_translate(struct atomisp_device *isp,
			      unsigned int *infos);

void atomisp_css_rx_get_irq_info(enum ia_css_csi2_port port,
					unsigned int *infos);

void atomisp_css_rx_clear_irq_info(enum ia_css_csi2_port port,
					unsigned int infos);

int atomisp_css_irq_enable(struct atomisp_device *isp,
			   enum atomisp_css_irq_info info, bool enable);

void atomisp_css_mmu_invalidate_tlb(void);

void atomisp_css_mmu_set_page_table_base_index(unsigned long base_index);

int atomisp_css_start(struct atomisp_sub_device *asd,
		      enum atomisp_css_pipe_id pipe_id, bool in_reset);

void atomisp_css_update_isp_params(struct atomisp_sub_device *asd);

void atomisp_css_isys_set_format(struct atomisp_sub_device *asd,
				 enum atomisp_input_stream_id stream_id,
				 enum atomisp_css_stream_format format,
				 int isys_stream);

int atomisp_css_set_default_isys_config(struct atomisp_sub_device *asd,
					enum atomisp_input_stream_id stream_id,

int atomisp_css_isys_two_stream_cfg(struct atomisp_sub_device *asd,
				    enum atomisp_input_stream_id stream_id,
				    enum atomisp_css_stream_format input_format);

void atomisp_css_isys_two_stream_cfg_update_stream1(
				    struct atomisp_sub_device *asd,
				    enum atomisp_input_stream_id stream_id,
				    enum atomisp_css_stream_format input_format,
				    unsigned int width, unsigned int height);

void atomisp_css_isys_two_stream_cfg_update_stream2(
				    struct atomisp_sub_device *asd,
				    enum atomisp_input_stream_id stream_id,
				    enum atomisp_css_stream_format input_format,
				    unsigned int width, unsigned int height);

int atomisp_css_input_set_resolution(struct atomisp_sub_device *asd,
					enum atomisp_input_stream_id stream_id,

void atomisp_css_input_set_format(struct atomisp_sub_device *asd,
				enum atomisp_input_stream_id stream_id,
				enum atomisp_css_stream_format format);

int atomisp_css_input_set_effective_resolution(
					struct atomisp_sub_device *asd,
					enum atomisp_input_stream_id stream_id,
							bool enable);

int atomisp_css_input_configure_port(struct atomisp_sub_device *asd,
				mipi_port_ID_t port,
				unsigned int num_lanes,
				unsigned int timeout,
				unsigned int mipi_freq,
				enum atomisp_css_stream_format metadata_format,
				unsigned int metadata_width,
				unsigned int metadata_height);

int atomisp_css_frame_allocate(struct atomisp_css_frame **frame,