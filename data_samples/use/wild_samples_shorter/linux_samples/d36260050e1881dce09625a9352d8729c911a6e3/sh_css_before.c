	assert(pipe->stream != NULL);

	switch (pipe->stream->config.input_config.format) {
	case IA_CSS_STREAM_FORMAT_YUV420_8_LEGACY:
	case IA_CSS_STREAM_FORMAT_YUV420_8:
		for (i=0; i<ARRAY_SIZE(yuv420_copy_formats) && !found; i++)
			found = (out_fmt == yuv420_copy_formats[i]);
		break;
	case IA_CSS_STREAM_FORMAT_YUV420_10:
	case IA_CSS_STREAM_FORMAT_YUV420_16:
		found = (out_fmt == IA_CSS_FRAME_FORMAT_YUV420_16);
		break;
	case IA_CSS_STREAM_FORMAT_YUV422_8:
		for (i=0; i<ARRAY_SIZE(yuv422_copy_formats) && !found; i++)
			found = (out_fmt == yuv422_copy_formats[i]);
		break;
	case IA_CSS_STREAM_FORMAT_YUV422_10:
	case IA_CSS_STREAM_FORMAT_YUV422_16:
		found = (out_fmt == IA_CSS_FRAME_FORMAT_YUV422_16 ||
			 out_fmt == IA_CSS_FRAME_FORMAT_YUV420_16);
		break;
	case IA_CSS_STREAM_FORMAT_RGB_444:
	case IA_CSS_STREAM_FORMAT_RGB_555:
	case IA_CSS_STREAM_FORMAT_RGB_565:
		found = (out_fmt == IA_CSS_FRAME_FORMAT_RGBA888 ||
			 out_fmt == IA_CSS_FRAME_FORMAT_RGB565);
		break;
	case IA_CSS_STREAM_FORMAT_RGB_666:
	case IA_CSS_STREAM_FORMAT_RGB_888:
		found = (out_fmt == IA_CSS_FRAME_FORMAT_RGBA888 ||
			 out_fmt == IA_CSS_FRAME_FORMAT_YUV420);
		break;
	case IA_CSS_STREAM_FORMAT_RAW_6:
	case IA_CSS_STREAM_FORMAT_RAW_7:
	case IA_CSS_STREAM_FORMAT_RAW_8:
	case IA_CSS_STREAM_FORMAT_RAW_10:
	case IA_CSS_STREAM_FORMAT_RAW_12:
	case IA_CSS_STREAM_FORMAT_RAW_14:
	case IA_CSS_STREAM_FORMAT_RAW_16:
		found = (out_fmt == IA_CSS_FRAME_FORMAT_RAW) ||
			(out_fmt == IA_CSS_FRAME_FORMAT_RAW_PACKED);
		break;
	case IA_CSS_STREAM_FORMAT_BINARY_8:
		found = (out_fmt == IA_CSS_FRAME_FORMAT_BINARY_8);
		break;
	default:
		break;
}
#elif !defined(HAS_NO_INPUT_SYSTEM) && defined(USE_INPUT_SYSTEM_VERSION_2401)
static unsigned int csi2_protocol_calculate_max_subpixels_per_line(
		enum ia_css_stream_format	format,
		unsigned int			pixels_per_line)
{
	unsigned int rval;

	switch (format) {
	case IA_CSS_STREAM_FORMAT_YUV420_8_LEGACY:
		/*
		 * The frame format layout is shown below.
		 *
		 *		Line	0:	UYY0 UYY0 ... UYY0
		 */
		rval = pixels_per_line * 2;
		break;
	case IA_CSS_STREAM_FORMAT_YUV420_8:
	case IA_CSS_STREAM_FORMAT_YUV420_10:
	case IA_CSS_STREAM_FORMAT_YUV420_16:
		/*
		 * The frame format layout is shown below.
		 *
		 *		Line	0:	YYYY YYYY ... YYYY
		 */
		rval = pixels_per_line * 2;
		break;
	case IA_CSS_STREAM_FORMAT_YUV422_8:
	case IA_CSS_STREAM_FORMAT_YUV422_10:
	case IA_CSS_STREAM_FORMAT_YUV422_16:
		/*
		 * The frame format layout is shown below.
		 *
		 *		Line	0:	UYVY UYVY ... UYVY
		 */
		rval = pixels_per_line * 2;
		break;
	case IA_CSS_STREAM_FORMAT_RGB_444:
	case IA_CSS_STREAM_FORMAT_RGB_555:
	case IA_CSS_STREAM_FORMAT_RGB_565:
	case IA_CSS_STREAM_FORMAT_RGB_666:
	case IA_CSS_STREAM_FORMAT_RGB_888:
		/*
		 * The frame format layout is shown below.
		 *
		 *		Line	0:	ABGR ABGR ... ABGR
		 */
		rval = pixels_per_line * 4;
		break;
	case IA_CSS_STREAM_FORMAT_RAW_6:
	case IA_CSS_STREAM_FORMAT_RAW_7:
	case IA_CSS_STREAM_FORMAT_RAW_8:
	case IA_CSS_STREAM_FORMAT_RAW_10:
	case IA_CSS_STREAM_FORMAT_RAW_12:
	case IA_CSS_STREAM_FORMAT_RAW_14:
	case IA_CSS_STREAM_FORMAT_RAW_16:
	case IA_CSS_STREAM_FORMAT_BINARY_8:
	case IA_CSS_STREAM_FORMAT_USER_DEF1:
	case IA_CSS_STREAM_FORMAT_USER_DEF2:
	case IA_CSS_STREAM_FORMAT_USER_DEF3:
	case IA_CSS_STREAM_FORMAT_USER_DEF4:
	case IA_CSS_STREAM_FORMAT_USER_DEF5:
	case IA_CSS_STREAM_FORMAT_USER_DEF6:
	case IA_CSS_STREAM_FORMAT_USER_DEF7:
	case IA_CSS_STREAM_FORMAT_USER_DEF8:
		/*
		 * The frame format layout is shown below.
		 *
		 *		Line	0:	Pixel Pixel ... Pixel
		break;
	case IA_CSS_INPUT_MODE_BUFFERED_SENSOR:

		if (stream_cfg->source.port.port == IA_CSS_CSI2_PORT0) {
			isys_stream_descr->input_port_id = INPUT_SYSTEM_CSI_PORT0_ID;
		} else if (stream_cfg->source.port.port == IA_CSS_CSI2_PORT1) {
			isys_stream_descr->input_port_id = INPUT_SYSTEM_CSI_PORT1_ID;
		} else if (stream_cfg->source.port.port == IA_CSS_CSI2_PORT2) {
			isys_stream_descr->input_port_id = INPUT_SYSTEM_CSI_PORT2_ID;
		}

		break;
	unsigned int max_subpixels_per_line;
	unsigned int lines_per_frame;
	unsigned int align_req_in_bytes;
	enum ia_css_stream_format fmt_type;

	fmt_type = stream_cfg->isys_config[isys_stream_idx].format;
	if ((stream_cfg->mode == IA_CSS_INPUT_MODE_SENSOR ||
			stream_cfg->mode == IA_CSS_INPUT_MODE_BUFFERED_SENSOR) &&

		if (stream_cfg->source.port.compression.uncompressed_bits_per_pixel ==
			UNCOMPRESSED_BITS_PER_PIXEL_10) {
				fmt_type = IA_CSS_STREAM_FORMAT_RAW_10;
		}
		else if (stream_cfg->source.port.compression.uncompressed_bits_per_pixel ==
			UNCOMPRESSED_BITS_PER_PIXEL_12) {
				fmt_type = IA_CSS_STREAM_FORMAT_RAW_12;
		}
		else
			return false;
	}

	/* get the SP thread id */
	rc = ia_css_pipeline_get_sp_thread_id(ia_css_pipe_get_pipe_num(pipe), &sp_thread_id);
	if (rc != true)
		return IA_CSS_ERR_INTERNAL_ERROR;
	/* get the target input terminal */
	sp_pipeline_input_terminal = &(sh_css_sp_group.pipe_io[sp_thread_id].input);

					&(isys_stream_descr));
		}

		if (rc != true)
			return IA_CSS_ERR_INTERNAL_ERROR;

		isys_stream_id = ia_css_isys_generate_stream_id(sp_thread_id, i);

				&(isys_stream_descr),
				&(sp_pipeline_input_terminal->context.virtual_input_system_stream[i]),
				isys_stream_id);
		if (rc != true)
			return IA_CSS_ERR_INTERNAL_ERROR;

		/* calculate the configuration of the virtual Input System (2401) */
		rc = ia_css_isys_stream_calculate_cfg(
				&(sp_pipeline_input_terminal->context.virtual_input_system_stream[i]),
				&(isys_stream_descr),
				&(sp_pipeline_input_terminal->ctrl.virtual_input_system_stream_cfg[i]));
		if (rc != true) {
			ia_css_isys_stream_destroy(&(sp_pipeline_input_terminal->context.virtual_input_system_stream[i]));
			return IA_CSS_ERR_INTERNAL_ERROR;
		}
	}

static enum ia_css_err stream_csi_rx_helper(
	struct ia_css_stream *stream,
	enum ia_css_err (*func)(enum ia_css_csi2_port, uint32_t))
{
	enum ia_css_err retval = IA_CSS_ERR_INTERNAL_ERROR;
	uint32_t sp_thread_id, stream_id;
	bool rc;
		ia_css_isys_rx_disable();
#endif

	if (pipe->stream->config.input_config.format != IA_CSS_STREAM_FORMAT_BINARY_8)
		return IA_CSS_ERR_INTERNAL_ERROR;
	sh_css_sp_start_binary_copy(ia_css_pipe_get_pipe_num(pipe), out_frame, pipe->stream->config.pixels_per_clock == 2);

#if !defined(HAS_NO_INPUT_SYSTEM) && !defined(USE_INPUT_SYSTEM_VERSION_2401)
				&me->stream->info.metadata_info
#if !defined(HAS_NO_INPUT_SYSTEM)
				,(input_mode==IA_CSS_INPUT_MODE_MEMORY) ?
					(mipi_port_ID_t)0 :
					me->stream->config.source.port.port
#endif
#ifdef ISP2401
				,&me->config.internal_frame_origin_bqs_on_sctbl,
enable_interrupts(enum ia_css_irq_type irq_type)
{
#ifdef USE_INPUT_SYSTEM_VERSION_2
	mipi_port_ID_t port;
#endif
	bool enable_pulse = irq_type != IA_CSS_IRQ_TYPE_EDGE;
	IA_CSS_ENTER_PRIVATE("");
	/* Enable IRQ on the SP which signals that SP goes to idle
	ifmtr_set_if_blocking_mode_reset = true;
#endif

	if (fw_explicitly_loaded == false) {
		ia_css_unload_firmware();
	}
	ia_css_spctrl_unload_fw(SP0_ID);
	sh_css_sp_set_sp_running(false);
#endif
#if !defined(HAS_NO_INPUT_SYSTEM)
#ifndef ISP2401
			, (mipi_port_ID_t)0
#else
			(mipi_port_ID_t)0,
#endif
#endif
#ifndef ISP2401
			);
#endif
#if !defined(HAS_NO_INPUT_SYSTEM)
#ifndef ISP2401
			, (mipi_port_ID_t) 0
#else
			(mipi_port_ID_t) 0,
#endif
#endif
#ifndef ISP2401
			);
	event->type = convert_event_sp_to_host_domain[payload[0]];
	/* Some sane default values since not all events use all fields. */
	event->pipe = NULL;
	event->port = IA_CSS_CSI2_PORT0;
	event->exp_id = 0;
	event->fw_warning = IA_CSS_FW_WARNING_NONE;
	event->fw_handle = 0;
	event->timer_data = 0;
		}
	}
	if (event->type == IA_CSS_EVENT_TYPE_PORT_EOF) {
		event->port = (enum ia_css_csi2_port)payload[1];
		event->exp_id = payload[3];
	} else if (event->type == IA_CSS_EVENT_TYPE_FW_WARNING) {
		event->fw_warning = (enum ia_css_fw_warning)payload[1];
		/* exp_id is only available in these warning types */
#endif
#if !defined(HAS_NO_INPUT_SYSTEM)
#ifndef ISP2401
			, (mipi_port_ID_t)0
#else
			(mipi_port_ID_t)0,
#endif
#endif
#ifndef ISP2401
			);

	rval &= (pipe->config.default_capture_config.mode == IA_CSS_CAPTURE_MODE_RAW);

	rval &= ((pipe->stream->config.input_config.format == IA_CSS_STREAM_FORMAT_BINARY_8) ||
		(pipe->config.mode == IA_CSS_PIPE_MODE_COPY));

	return rval;
}
		return err;
	}
	if (copy_on_sp(pipe) &&
	    pipe->stream->config.input_config.format == IA_CSS_STREAM_FORMAT_BINARY_8) {
		ia_css_frame_info_init(
			&pipe->output_info[0],
			JPEG_BYTES,
			1,

	/* TODO: make generic function */
	need_format_conversion =
		((pipe->stream->config.input_config.format == IA_CSS_STREAM_FORMAT_YUV420_8_LEGACY) &&
		(pipe->output_info[0].format != IA_CSS_FRAME_FORMAT_CSI_MIPI_LEGACY_YUV420_8));

	in_res = pipe->config.input_effective_res;

	/*
	 * NOTES
	 * - Why does the "yuvpp" pipe needs "isp_copy_binary" (i.e. ISP Copy) when
	 *   its input is "IA_CSS_STREAM_FORMAT_YUV422_8"?
	 *
	 *   In most use cases, the first stage in the "yuvpp" pipe is the "yuv_scale_
	 *   binary". However, the "yuv_scale_binary" does NOT support the input-frame
	 *   format as "IA_CSS_STREAM _FORMAT_YUV422_8".
	 *   "yuv_scale_binary".
	 */
	need_isp_copy_binary =
		(pipe->stream->config.input_config.format == IA_CSS_STREAM_FORMAT_YUV422_8);
#else  /* !USE_INPUT_SYSTEM_VERSION_2401 */
	need_isp_copy_binary = true;
#endif /*  USE_INPUT_SYSTEM_VERSION_2401 */

		 * Bayer-Quad RAW.
		 */
		int in_frame_format;
		if (pipe->stream->config.input_config.format == IA_CSS_STREAM_FORMAT_YUV420_8_LEGACY) {
			in_frame_format = IA_CSS_FRAME_FORMAT_CSI_MIPI_LEGACY_YUV420_8;
		} else if (pipe->stream->config.input_config.format == IA_CSS_STREAM_FORMAT_YUV422_8) {
			/*
			 * When the sensor output frame format is "IA_CSS_STREAM_FORMAT_YUV422_8",
			 * the "isp_copy_var" binary is selected as the first stage in the yuvpp
			 * pipe.
			 *
			 * For the "isp_copy_var" binary, it reads the YUV422-8 pixels from

		for (i = 0, j = 0; i < num_stage; i++) {
			assert(j < num_output_stage);
			if (pipe->pipe_settings.yuvpp.is_output_stage[i] == true) {
				tmp_out_frame = out_frame[j];
				tmp_vf_frame = vf_frame[j];
			} else {
				tmp_out_frame = NULL;
			}
			/* we use output port 1 as internal output port */
			tmp_in_frame = yuv_scaler_stage->args.out_frame[1];
			if (pipe->pipe_settings.yuvpp.is_output_stage[i] == true) {
				if (tmp_vf_frame && (tmp_vf_frame->info.res.width != 0)) {
					in_frame = yuv_scaler_stage->args.out_vf_frame;
					err = add_vf_pp_stage(pipe, in_frame, tmp_vf_frame, &vf_pp_binary[j],
						      &vf_pp_stage);
	out_frame->flash_state = IA_CSS_FRAME_FLASH_STATE_NONE;

	if (copy_on_sp(pipe) &&
	    pipe->stream->config.input_config.format == IA_CSS_STREAM_FORMAT_BINARY_8) {
		ia_css_frame_info_init(
			&out_frame->info,
			JPEG_BYTES,
			1,
	}

	if (mode == IA_CSS_CAPTURE_MODE_PRIMARY) {
		unsigned int frm;
		struct ia_css_frame *local_in_frame = NULL;
		struct ia_css_frame *local_out_frame = NULL;

		for (i = 0; i < num_primary_stage; i++) {
				return err;
			}
		}
		(void)frm;
		/* If we use copy iso primary,
		   the input must be yuv iso raw */
		current_stage->args.copy_vf =
			primary_binary[0]->info->sp.pipeline.mode ==
				  struct ia_css_frame_info *info,
				  unsigned int idx)
{
	enum ia_css_err err = IA_CSS_SUCCESS;

	assert(pipe != NULL);
	assert(info != NULL);

	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE_PRIVATE,

	*info = pipe->output_info[idx];
	if (copy_on_sp(pipe) &&
	    pipe->stream->config.input_config.format == IA_CSS_STREAM_FORMAT_BINARY_8) {
		ia_css_frame_info_init(
			info,
			JPEG_BYTES,
			1,

	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE_PRIVATE,
						"sh_css_pipe_get_output_frame_info() leave:\n");
	return err;
}

#if !defined(HAS_NO_INPUT_SYSTEM)
void

void
ia_css_stream_send_input_embedded_line(const struct ia_css_stream *stream,
		enum ia_css_stream_format format,
		const unsigned short *data,
		unsigned int width)
{
	assert(stream != NULL);
	else if (config->num_lanes != 0)
		return IA_CSS_ERR_INVALID_ARGUMENTS;

	if (config->port > IA_CSS_CSI2_PORT2)
		return IA_CSS_ERR_INVALID_ARGUMENTS;
	stream->csi_rx_config.port =
		ia_css_isys_port_to_mipi_port(config->port);
	stream->csi_rx_config.timeout    = config->timeout;

#if defined(USE_INPUT_SYSTEM_VERSION_2)
	/* We don't support metadata for JPEG stream, since they both use str2mem */
	if (stream_config->input_config.format == IA_CSS_STREAM_FORMAT_BINARY_8 &&
	    stream_config->metadata_config.resolution.height > 0) {
		err = IA_CSS_ERR_INVALID_ARGUMENTS;
		IA_CSS_LEAVE_ERR(err);
		return err;
	return IA_CSS_SUCCESS;
}

enum ia_css_stream_format
ia_css_stream_get_format(const struct ia_css_stream *stream)
{
	return stream->config.input_config.format;
}
enum ia_css_err
ia_css_stream_set_output_padded_width(struct ia_css_stream *stream, unsigned int output_padded_width)
{
	enum ia_css_err err = IA_CSS_SUCCESS;

	struct ia_css_pipe *pipe;

	assert(stream != NULL);

	pipe->config.output_info[IA_CSS_PIPE_OUTPUT_STAGE_0].padded_width = output_padded_width;
	pipe->output_info[IA_CSS_PIPE_OUTPUT_STAGE_0].padded_width = output_padded_width;

	return err;
}

static struct ia_css_binary *
ia_css_pipe_get_shading_correction_binary(const struct ia_css_pipe *pipe)
				(uint8_t) IA_CSS_PSYS_SW_EVENT_STAGE_ENABLE_DISABLE,
				(uint8_t) thread_id,
				(uint8_t) stage->stage_num,
				(enable == true) ? 1 : 0);
			if (err == IA_CSS_SUCCESS) {
				if(enable)
					SH_CSS_QOS_STAGE_ENABLE(&(sh_css_sp_group.pipe[thread_id]),stage->stage_num);
				else

	buffer_record = &hmm_buffer_record[0];
	for (i = 0; i < MAX_HMM_BUFFER_NUM; i++) {
		if (buffer_record->in_use == false) {
			buffer_record->in_use = true;
			buffer_record->type = type;
			buffer_record->h_vbuf = h_vbuf;
			buffer_record->kernel_ptr = kernel_ptr;

	buffer_record = &hmm_buffer_record[0];
	for (i = 0; i < MAX_HMM_BUFFER_NUM; i++) {
		if ((buffer_record->in_use == true) &&
		    (buffer_record->type == type) &&
		    (buffer_record->h_vbuf != NULL) &&
		    (buffer_record->h_vbuf->vptr == ddr_buffer_addr)) {
			found_record = true;
		buffer_record++;
	}

	if (found_record == true)
		return buffer_record;
	else
		return NULL;
}