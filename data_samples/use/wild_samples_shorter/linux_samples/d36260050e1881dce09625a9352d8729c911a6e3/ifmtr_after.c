	    width_b_factor = 1, start_column_b,
	    left_padding = 0;
	input_formatter_cfg_t if_a_config, if_b_config;
	enum atomisp_input_format input_format;
	enum ia_css_err err = IA_CSS_SUCCESS;
	uint8_t if_config_index;

	/* Determine which input formatter config set is targeted. */
	/* Index is equal to the CSI-2 port used. */
	enum mipi_port_id port;

	if (binary) {
		cropped_height = binary->in_frame_info.res.height;
		cropped_width = binary->in_frame_info.res.width;
	if (config->mode == IA_CSS_INPUT_MODE_SENSOR
	    || config->mode == IA_CSS_INPUT_MODE_BUFFERED_SENSOR) {
		port = config->source.port.port;
		if_config_index = (uint8_t) (port - MIPI_PORT0_ID);
	} else if (config->mode == IA_CSS_INPUT_MODE_MEMORY) {
		if_config_index = SH_CSS_IF_CONFIG_NOT_NEEDED;
	} else {
		if_config_index = 0;
	bits_per_pixel = input_formatter_get_alignment(INPUT_FORMATTER0_ID)
	    * 8 / ISP_VEC_NELEMS;
	switch (input_format) {
	case ATOMISP_INPUT_FORMAT_YUV420_8_LEGACY:
		if (two_ppc) {
			vmem_increment = 1;
			deinterleaving = 1;
			deinterleaving_b = 1;
			start_column = start_column * deinterleaving / 2;
		}
		break;
	case ATOMISP_INPUT_FORMAT_YUV420_8:
	case ATOMISP_INPUT_FORMAT_YUV420_10:
	case ATOMISP_INPUT_FORMAT_YUV420_16:
		if (two_ppc) {
			vmem_increment = 1;
			deinterleaving = 1;
			width_a = width_b = cropped_width * deinterleaving / 2;
			start_column *= deinterleaving;
		}
		break;
	case ATOMISP_INPUT_FORMAT_YUV422_8:
	case ATOMISP_INPUT_FORMAT_YUV422_10:
	case ATOMISP_INPUT_FORMAT_YUV422_16:
		if (two_ppc) {
			vmem_increment = 1;
			deinterleaving = 1;
			width_a = width_b = cropped_width * deinterleaving;
			start_column *= deinterleaving;
		}
		break;
	case ATOMISP_INPUT_FORMAT_RGB_444:
	case ATOMISP_INPUT_FORMAT_RGB_555:
	case ATOMISP_INPUT_FORMAT_RGB_565:
	case ATOMISP_INPUT_FORMAT_RGB_666:
	case ATOMISP_INPUT_FORMAT_RGB_888:
		num_vectors *= 2;
		if (two_ppc) {
			deinterleaving = 2;	/* BR in if_a, G in if_b */
			deinterleaving_b = 1;	/* BR in if_a, G in if_b */
		num_vectors = num_vectors / 2 * deinterleaving;
		buf_offset_b = buffer_width / 2 / ISP_VEC_NELEMS;
		break;
	case ATOMISP_INPUT_FORMAT_RAW_6:
	case ATOMISP_INPUT_FORMAT_RAW_7:
	case ATOMISP_INPUT_FORMAT_RAW_8:
	case ATOMISP_INPUT_FORMAT_RAW_10:
	case ATOMISP_INPUT_FORMAT_RAW_12:
		if (two_ppc) {
			int crop_col = (start_column % 2) == 1;
			vmem_increment = 2;
			deinterleaving = 1;
		vectors_per_line = CEIL_DIV(cropped_width, ISP_VEC_NELEMS);
		vectors_per_line = CEIL_MUL(vectors_per_line, deinterleaving);
		break;
	case ATOMISP_INPUT_FORMAT_RAW_14:
	case ATOMISP_INPUT_FORMAT_RAW_16:
		if (two_ppc) {
			num_vectors *= 2;
			vmem_increment = 1;
			deinterleaving = 2;
		}
		buffer_height *= 2;
		break;
	case ATOMISP_INPUT_FORMAT_BINARY_8:
	case ATOMISP_INPUT_FORMAT_GENERIC_SHORT1:
	case ATOMISP_INPUT_FORMAT_GENERIC_SHORT2:
	case ATOMISP_INPUT_FORMAT_GENERIC_SHORT3:
	case ATOMISP_INPUT_FORMAT_GENERIC_SHORT4:
	case ATOMISP_INPUT_FORMAT_GENERIC_SHORT5:
	case ATOMISP_INPUT_FORMAT_GENERIC_SHORT6:
	case ATOMISP_INPUT_FORMAT_GENERIC_SHORT7:
	case ATOMISP_INPUT_FORMAT_GENERIC_SHORT8:
	case ATOMISP_INPUT_FORMAT_YUV420_8_SHIFT:
	case ATOMISP_INPUT_FORMAT_YUV420_10_SHIFT:
	case ATOMISP_INPUT_FORMAT_EMBEDDED:
	case ATOMISP_INPUT_FORMAT_USER_DEF1:
	case ATOMISP_INPUT_FORMAT_USER_DEF2:
	case ATOMISP_INPUT_FORMAT_USER_DEF3:
	case ATOMISP_INPUT_FORMAT_USER_DEF4:
	case ATOMISP_INPUT_FORMAT_USER_DEF5:
	case ATOMISP_INPUT_FORMAT_USER_DEF6:
	case ATOMISP_INPUT_FORMAT_USER_DEF7:
	case ATOMISP_INPUT_FORMAT_USER_DEF8:
		break;
	}
	if (width_a == 0)
		return IA_CSS_ERR_INVALID_ARGUMENTS;
	if_a_config.buf_eol_offset =
	    buffer_width * bits_per_pixel / 8 - line_width;
	if_a_config.is_yuv420_format =
	    (input_format == ATOMISP_INPUT_FORMAT_YUV420_8)
	    || (input_format == ATOMISP_INPUT_FORMAT_YUV420_10)
	    || (input_format == ATOMISP_INPUT_FORMAT_YUV420_16);
	if_a_config.block_no_reqs = (config->mode != IA_CSS_INPUT_MODE_SENSOR);

	if (two_ppc) {
		if (deinterleaving_b) {
		if_b_config.buf_eol_offset =
		    buffer_width * bits_per_pixel / 8 - line_width;
		if_b_config.is_yuv420_format =
		    input_format == ATOMISP_INPUT_FORMAT_YUV420_8
		    || input_format == ATOMISP_INPUT_FORMAT_YUV420_10
		    || input_format == ATOMISP_INPUT_FORMAT_YUV420_16;
		if_b_config.block_no_reqs =
		    (config->mode != IA_CSS_INPUT_MODE_SENSOR);

		if (SH_CSS_IF_CONFIG_NOT_NEEDED != if_config_index) {