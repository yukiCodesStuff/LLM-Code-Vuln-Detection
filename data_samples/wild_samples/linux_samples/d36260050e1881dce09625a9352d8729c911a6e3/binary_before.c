	for (i = 0; i < info->num_output_formats; i++) {
		if (info->output_formats[i] == format)
			return true;
	}
	return false;
}

#ifdef ISP2401
static bool
binary_supports_input_format(const struct ia_css_binary_xinfo *info,
			     enum ia_css_stream_format format)
{

	assert(info != NULL);
	(void)format;

	return true;
}
	} else {
		rval = isp_internal_width;
	}

	return rval;
}


enum ia_css_err
ia_css_binary_fill_info(const struct ia_css_binary_xinfo *xinfo,
		 bool online,
		 bool two_ppc,
		 enum ia_css_stream_format stream_format,
		 const struct ia_css_frame_info *in_info, /* can be NULL */
		 const struct ia_css_frame_info *bds_out_info, /* can be NULL */
		 const struct ia_css_frame_info *out_info[], /* can be NULL */
		 const struct ia_css_frame_info *vf_info, /* can be NULL */
		 struct ia_css_binary *binary,
		 struct ia_css_resolution *dvs_env,
		 int stream_config_left_padding,
		 bool accelerator)
{
	const struct ia_css_binary_info *info = &xinfo->sp;
	unsigned int dvs_env_width = 0,
		     dvs_env_height = 0,
		     vf_log_ds = 0,
		     s3a_log_deci = 0,
		     bits_per_pixel = 0,
		     /* Resolution at SC/3A/DIS kernel. */
		     sc_3a_dis_width = 0,
		     /* Resolution at SC/3A/DIS kernel. */
		     sc_3a_dis_padded_width = 0,
		     /* Resolution at SC/3A/DIS kernel. */
		     sc_3a_dis_height = 0,
		     isp_internal_width = 0,
		     isp_internal_height = 0,
		     s3a_isp_width = 0;

	bool need_scaling = false;
	struct ia_css_resolution binary_dvs_env, internal_res;
	enum ia_css_err err;
	unsigned int i;
	const struct ia_css_frame_info *bin_out_info = NULL;

	assert(info != NULL);
	assert(binary != NULL);

	binary->info = xinfo;
	if (!accelerator) {
		/* binary->css_params has been filled by accelerator itself. */
		err = ia_css_isp_param_allocate_isp_parameters(
			&binary->mem_params, &binary->css_params,
			&info->mem_initializers);
		if (err != IA_CSS_SUCCESS) {
			return err;
		}
	}
	for (i = 0; i < IA_CSS_BINARY_MAX_OUTPUT_PORTS; i++) {
		if (out_info[i] && (out_info[i]->res.width != 0)) {
			bin_out_info = out_info[i];
			break;
		}
	}
	if (in_info != NULL && bin_out_info != NULL) {
		need_scaling = (in_info->res.width != bin_out_info->res.width) ||
			(in_info->res.height != bin_out_info->res.height);
	}


	/* binary_dvs_env has to be equal or larger than SH_CSS_MIN_DVS_ENVELOPE */
	binary_dvs_env.width = 0;
	binary_dvs_env.height = 0;
	ia_css_binary_dvs_env(info, dvs_env, &binary_dvs_env);
	dvs_env_width = binary_dvs_env.width;
	dvs_env_height = binary_dvs_env.height;
	binary->dvs_envelope.width  = dvs_env_width;
	binary->dvs_envelope.height = dvs_env_height;

	/* internal resolution calculation */
	internal_res.width = 0;
	internal_res.height = 0;
	ia_css_binary_internal_res(in_info, bds_out_info, bin_out_info, dvs_env,
				   info, &internal_res);
	isp_internal_width = internal_res.width;
	isp_internal_height = internal_res.height;

	/* internal frame info */
	if (bin_out_info != NULL) /* { */
		binary->internal_frame_info.format = bin_out_info->format;
	/* } */
	binary->internal_frame_info.res.width       = isp_internal_width;
	binary->internal_frame_info.padded_width    = CEIL_MUL(isp_internal_width, 2*ISP_VEC_NELEMS);
	binary->internal_frame_info.res.height      = isp_internal_height;
	binary->internal_frame_info.raw_bit_depth   = bits_per_pixel;

	if (in_info != NULL) {
		binary->effective_in_frame_res.width = in_info->res.width;
		binary->effective_in_frame_res.height = in_info->res.height;

		bits_per_pixel = in_info->raw_bit_depth;

		/* input info */
		binary->in_frame_info.res.width = in_info->res.width + info->pipeline.left_cropping;
		binary->in_frame_info.res.height = in_info->res.height + info->pipeline.top_cropping;

		binary->in_frame_info.res.width += dvs_env_width;
		binary->in_frame_info.res.height += dvs_env_height;

		binary->in_frame_info.padded_width =
			binary_in_frame_padded_width(in_info->res.width,
						     isp_internal_width,
						     dvs_env_width,
						     stream_config_left_padding,
						     info->pipeline.left_cropping,
						     need_scaling);

		binary->in_frame_info.format = in_info->format;
		binary->in_frame_info.raw_bayer_order = in_info->raw_bayer_order;
		binary->in_frame_info.crop_info = in_info->crop_info;
	}

	if (online) {
		bits_per_pixel = ia_css_util_input_format_bpp(
			stream_format, two_ppc);
	}
	binary->in_frame_info.raw_bit_depth = bits_per_pixel;

	for (i = 0; i < IA_CSS_BINARY_MAX_OUTPUT_PORTS; i++) {
		if (out_info[i] != NULL) {
			binary->out_frame_info[i].res.width     = out_info[i]->res.width;
			binary->out_frame_info[i].res.height    = out_info[i]->res.height;
			binary->out_frame_info[i].padded_width  = out_info[i]->padded_width;
			if (info->pipeline.mode == IA_CSS_BINARY_MODE_COPY) {
				binary->out_frame_info[i].raw_bit_depth = bits_per_pixel;
			} else {
				/* Only relevant for RAW format.
				 * At the moment, all outputs are raw, 16 bit per pixel, except for copy.
				 * To do this cleanly, the binary should specify in its info
				 * the bit depth per output channel.
				 */
				binary->out_frame_info[i].raw_bit_depth = 16;
			}
			binary->out_frame_info[i].format        = out_info[i]->format;
		}
	}

	if (vf_info && (vf_info->res.width != 0)) {
		err = ia_css_vf_configure(binary, bin_out_info, (struct ia_css_frame_info *)vf_info, &vf_log_ds);
		if (err != IA_CSS_SUCCESS) {
			if (!accelerator) {
				ia_css_isp_param_destroy_isp_parameters(
					&binary->mem_params,
					&binary->css_params);
			}
			return err;
		}
	}
	binary->vf_downscale_log2 = vf_log_ds;

	binary->online            = online;
	binary->input_format      = stream_format;

	/* viewfinder output info */
	if ((vf_info != NULL) && (vf_info->res.width != 0)) {
		unsigned int vf_out_vecs, vf_out_width, vf_out_height;
		binary->vf_frame_info.format = vf_info->format;
		if (bin_out_info == NULL)
			return IA_CSS_ERR_INTERNAL_ERROR;
		vf_out_vecs = __ISP_VF_OUTPUT_WIDTH_VECS(bin_out_info->padded_width,
			vf_log_ds);
		vf_out_width = _ISP_VF_OUTPUT_WIDTH(vf_out_vecs);
		vf_out_height = _ISP_VF_OUTPUT_HEIGHT(bin_out_info->res.height,
			vf_log_ds);

		/* For preview mode, output pin is used instead of vf. */
		if (info->pipeline.mode == IA_CSS_BINARY_MODE_PREVIEW) {
			binary->out_frame_info[0].res.width =
				(bin_out_info->res.width >> vf_log_ds);
			binary->out_frame_info[0].padded_width = vf_out_width;
			binary->out_frame_info[0].res.height   = vf_out_height;

			binary->vf_frame_info.res.width    = 0;
			binary->vf_frame_info.padded_width = 0;
			binary->vf_frame_info.res.height   = 0;
		} else {
			/* we also store the raw downscaled width. This is
			 * used for digital zoom in preview to zoom only on
			 * the width that we actually want to keep, not on
			 * the aligned width. */
			binary->vf_frame_info.res.width =
				(bin_out_info->res.width >> vf_log_ds);
			binary->vf_frame_info.padded_width = vf_out_width;
			binary->vf_frame_info.res.height   = vf_out_height;
		}
	} else {
		binary->vf_frame_info.res.width    = 0;
		binary->vf_frame_info.padded_width = 0;
		binary->vf_frame_info.res.height   = 0;
	}

	if (info->enable.ca_gdc) {
		binary->morph_tbl_width =
			_ISP_MORPH_TABLE_WIDTH(isp_internal_width);
		binary->morph_tbl_aligned_width  =
			_ISP_MORPH_TABLE_ALIGNED_WIDTH(isp_internal_width);
		binary->morph_tbl_height =
			_ISP_MORPH_TABLE_HEIGHT(isp_internal_height);
	} else {
		binary->morph_tbl_width  = 0;
		binary->morph_tbl_aligned_width  = 0;
		binary->morph_tbl_height = 0;
	}

	sc_3a_dis_width = binary->in_frame_info.res.width;
	sc_3a_dis_padded_width = binary->in_frame_info.padded_width;
	sc_3a_dis_height = binary->in_frame_info.res.height;
	if (bds_out_info != NULL && in_info != NULL &&
			bds_out_info->res.width != in_info->res.width) {
		/* TODO: Next, "internal_frame_info" should be derived from
		 * bds_out. So this part will change once it is in place! */
		sc_3a_dis_width = bds_out_info->res.width + info->pipeline.left_cropping;
		sc_3a_dis_padded_width = isp_internal_width;
		sc_3a_dis_height = isp_internal_height;
	}


	s3a_isp_width = _ISP_S3A_ELEMS_ISP_WIDTH(sc_3a_dis_padded_width,
		info->pipeline.left_cropping);
	if (info->s3a.fixed_s3a_deci_log) {
		s3a_log_deci = info->s3a.fixed_s3a_deci_log;
	} else {
		s3a_log_deci = binary_grid_deci_factor_log2(s3a_isp_width,
							    sc_3a_dis_height);
	}
	binary->deci_factor_log2  = s3a_log_deci;

	if (info->enable.s3a) {
		binary->s3atbl_width  =
			_ISP_S3ATBL_WIDTH(sc_3a_dis_width,
				s3a_log_deci);
		binary->s3atbl_height =
			_ISP_S3ATBL_HEIGHT(sc_3a_dis_height,
				s3a_log_deci);
		binary->s3atbl_isp_width =
			_ISP_S3ATBL_ISP_WIDTH(s3a_isp_width,
					s3a_log_deci);
		binary->s3atbl_isp_height =
			_ISP_S3ATBL_ISP_HEIGHT(sc_3a_dis_height,
				s3a_log_deci);
	} else {
		binary->s3atbl_width  = 0;
		binary->s3atbl_height = 0;
		binary->s3atbl_isp_width  = 0;
		binary->s3atbl_isp_height = 0;
	}

	if (info->enable.sc) {
		binary->sctbl_width_per_color  =
#ifndef ISP2401
			_ISP_SCTBL_WIDTH_PER_COLOR(sc_3a_dis_padded_width,
				s3a_log_deci);
#else
			_ISP_SCTBL_WIDTH_PER_COLOR(isp_internal_width, s3a_log_deci);
#endif
		binary->sctbl_aligned_width_per_color =
			SH_CSS_MAX_SCTBL_ALIGNED_WIDTH_PER_COLOR;
		binary->sctbl_height =
#ifndef ISP2401
			_ISP_SCTBL_HEIGHT(sc_3a_dis_height, s3a_log_deci);
#else
			_ISP_SCTBL_HEIGHT(isp_internal_height, s3a_log_deci);
		binary->sctbl_legacy_width_per_color  =
			_ISP_SCTBL_LEGACY_WIDTH_PER_COLOR(sc_3a_dis_padded_width, s3a_log_deci);
		binary->sctbl_legacy_height =
			_ISP_SCTBL_LEGACY_HEIGHT(sc_3a_dis_height, s3a_log_deci);
#endif
	} else {
		binary->sctbl_width_per_color         = 0;
		binary->sctbl_aligned_width_per_color = 0;
		binary->sctbl_height                  = 0;
#ifdef ISP2401
		binary->sctbl_legacy_width_per_color  = 0;
		binary->sctbl_legacy_height	      = 0;
#endif
	}
	ia_css_sdis_init_info(&binary->dis,
				sc_3a_dis_width,
				sc_3a_dis_padded_width,
				sc_3a_dis_height,
				info->pipeline.isp_pipe_version,
				info->enable.dis);
	if (info->pipeline.left_cropping)
		binary->left_padding = 2 * ISP_VEC_NELEMS - info->pipeline.left_cropping;
	else
		binary->left_padding = 0;

	return IA_CSS_SUCCESS;
}

enum ia_css_err
ia_css_binary_find(struct ia_css_binary_descr *descr,
		   struct ia_css_binary *binary)
{
	int mode;
	bool online;
	bool two_ppc;
	enum ia_css_stream_format stream_format;
	const struct ia_css_frame_info *req_in_info,
				       *req_bds_out_info,
				       *req_out_info[IA_CSS_BINARY_MAX_OUTPUT_PORTS],
				       *req_bin_out_info = NULL,
				       *req_vf_info;

	struct ia_css_binary_xinfo *xcandidate;
#ifndef ISP2401
	bool need_ds, need_dz, need_dvs, need_xnr, need_dpc;
#else
	bool need_ds, need_dz, need_dvs, need_xnr, need_dpc, need_tnr;
#endif
	bool striped;
	bool enable_yuv_ds;
	bool enable_high_speed;
	bool enable_dvs_6axis;
	bool enable_reduced_pipe;
	bool enable_capture_pp_bli;
#ifdef ISP2401
	bool enable_luma_only;
#endif
	enum ia_css_err err = IA_CSS_ERR_INTERNAL_ERROR;
	bool continuous;
	unsigned int isp_pipe_version;
	struct ia_css_resolution dvs_env, internal_res;
	unsigned int i;

	assert(descr != NULL);
	/* MW: used after an error check, may accept NULL, but doubtfull */
	assert(binary != NULL);

	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
		"ia_css_binary_find() enter: descr=%p, (mode=%d), binary=%p\n",
		descr, descr->mode,
		binary);

	mode = descr->mode;
	online = descr->online;
	two_ppc = descr->two_ppc;
	stream_format = descr->stream_format;
	req_in_info = descr->in_info;
	req_bds_out_info = descr->bds_out_info;
	for (i = 0; i < IA_CSS_BINARY_MAX_OUTPUT_PORTS; i++) {
		req_out_info[i] = descr->out_info[i];
		if (req_out_info[i] && (req_out_info[i]->res.width != 0))
			req_bin_out_info = req_out_info[i];
	}
	if (req_bin_out_info == NULL)
		return IA_CSS_ERR_INTERNAL_ERROR;
#ifndef ISP2401
	req_vf_info = descr->vf_info;
#else

	if ((descr->vf_info != NULL) && (descr->vf_info->res.width == 0))
		/* width==0 means that there is no vf pin (e.g. in SkyCam preview case) */
		req_vf_info = NULL;
	else
		req_vf_info = descr->vf_info;
#endif

	need_xnr = descr->enable_xnr;
	need_ds = descr->enable_fractional_ds;
	need_dz = false;
	need_dvs = false;
	need_dpc = descr->enable_dpc;
#ifdef ISP2401
	need_tnr = descr->enable_tnr;
#endif
	enable_yuv_ds = descr->enable_yuv_ds;
	enable_high_speed = descr->enable_high_speed;
	enable_dvs_6axis  = descr->enable_dvs_6axis;
	enable_reduced_pipe = descr->enable_reduced_pipe;
	enable_capture_pp_bli = descr->enable_capture_pp_bli;
#ifdef ISP2401
	enable_luma_only = descr->enable_luma_only;
#endif
	continuous = descr->continuous;
	striped = descr->striped;
	isp_pipe_version = descr->isp_pipe_version;

	dvs_env.width = 0;
	dvs_env.height = 0;
	internal_res.width = 0;
	internal_res.height = 0;


	if (mode == IA_CSS_BINARY_MODE_VIDEO) {
		dvs_env = descr->dvs_env;
		need_dz = descr->enable_dz;
		/* Video is the only mode that has a nodz variant. */
		need_dvs = dvs_env.width || dvs_env.height;
	}

	/* print a map of the binary file */
	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,	"BINARY INFO:\n");
	for (i = 0; i < IA_CSS_BINARY_NUM_MODES; i++) {
		xcandidate = binary_infos[i];
		if (xcandidate) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,	"%d:\n", i);
			while (xcandidate) {
				ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE, " Name:%s Type:%d Cont:%d\n",
						xcandidate->blob->name, xcandidate->type,
						xcandidate->sp.enable.continuous);
				xcandidate = xcandidate->next;
			}
		}
	}

	/* printf("sh_css_binary_find: pipe version %d\n", isp_pipe_version); */
	for (xcandidate = binary_infos[mode]; xcandidate;
	     xcandidate = xcandidate->next) {
		struct ia_css_binary_info *candidate = &xcandidate->sp;
		/* printf("sh_css_binary_find: evaluating candidate:
		 * %d\n",candidate->id); */
		ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
			"ia_css_binary_find() candidate = %p, mode = %d ID = %d\n",
			candidate, candidate->pipeline.mode, candidate->id);

		/*
		 * MW: Only a limited set of jointly configured binaries can
		 * be used in a continuous preview/video mode unless it is
		 * the copy mode and runs on SP.
		*/
		if (!candidate->enable.continuous &&
		    continuous && (mode != IA_CSS_BINARY_MODE_COPY)) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: !%d && %d && (%d != %d)\n",
					__LINE__, candidate->enable.continuous,
					continuous, mode,
					IA_CSS_BINARY_MODE_COPY);
			continue;
		}
		if (striped && candidate->iterator.num_stripes == 1) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: binary is not striped\n",
					__LINE__);
			continue;
		}

		if (candidate->pipeline.isp_pipe_version != isp_pipe_version &&
		    (mode != IA_CSS_BINARY_MODE_COPY) &&
		    (mode != IA_CSS_BINARY_MODE_CAPTURE_PP) &&
		    (mode != IA_CSS_BINARY_MODE_VF_PP)) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: (%d != %d)\n",
				__LINE__,
				candidate->pipeline.isp_pipe_version, isp_pipe_version);
			continue;
		}
		if (!candidate->enable.reduced_pipe && enable_reduced_pipe) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: !%d && %d\n",
				__LINE__,
				candidate->enable.reduced_pipe,
				enable_reduced_pipe);
			continue;
		}
		if (!candidate->enable.dvs_6axis && enable_dvs_6axis) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: !%d && %d\n",
				__LINE__,
				candidate->enable.dvs_6axis,
				enable_dvs_6axis);
			continue;
		}
		if (candidate->enable.high_speed && !enable_high_speed) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: %d && !%d\n",
				__LINE__,
				candidate->enable.high_speed,
				enable_high_speed);
			continue;
		}
		if (!candidate->enable.xnr && need_xnr) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: %d && !%d\n",
				__LINE__,
				candidate->enable.xnr,
				need_xnr);
			continue;
		}
		if (!(candidate->enable.ds & 2) && enable_yuv_ds) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: !%d && %d\n",
				__LINE__,
				((candidate->enable.ds & 2) != 0),
				enable_yuv_ds);
			continue;
		}
		if ((candidate->enable.ds & 2) && !enable_yuv_ds) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: %d && !%d\n",
				__LINE__,
				((candidate->enable.ds & 2) != 0),
				enable_yuv_ds);
			continue;
		}

		if (mode == IA_CSS_BINARY_MODE_VIDEO &&
			candidate->enable.ds && need_ds)
			need_dz = false;

		/* when we require vf output, we need to have vf_veceven */
		if ((req_vf_info != NULL) && !(candidate->enable.vf_veceven ||
				/* or variable vf vec even */
				candidate->vf_dec.is_variable ||
				/* or more than one output pin. */
				xcandidate->num_output_pins > 1)) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: (%p != NULL) && !(%d || %d || (%d >%d))\n",
				__LINE__, req_vf_info,
				candidate->enable.vf_veceven,
				candidate->vf_dec.is_variable,
				xcandidate->num_output_pins, 1);
			continue;
		}
		if (!candidate->enable.dvs_envelope && need_dvs) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: !%d && %d\n",
				__LINE__,
				candidate->enable.dvs_envelope, (int)need_dvs);
			continue;
		}
		/* internal_res check considers input, output, and dvs envelope sizes */
		ia_css_binary_internal_res(req_in_info, req_bds_out_info,
					   req_bin_out_info, &dvs_env, candidate, &internal_res);
		if (internal_res.width > candidate->internal.max_width) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
			"ia_css_binary_find() [%d] continue: (%d > %d)\n",
			__LINE__, internal_res.width,
			candidate->internal.max_width);
			continue;
		}
		if (internal_res.height > candidate->internal.max_height) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
			"ia_css_binary_find() [%d] continue: (%d > %d)\n",
			__LINE__, internal_res.height,
			candidate->internal.max_height);
			continue;
		}
		if (!candidate->enable.ds && need_ds && !(xcandidate->num_output_pins > 1)) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: !%d && %d\n",
				__LINE__, candidate->enable.ds, (int)need_ds);
			continue;
		}
		if (!candidate->enable.uds && !candidate->enable.dvs_6axis && need_dz) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: !%d && !%d && %d\n",
				__LINE__, candidate->enable.uds,
				candidate->enable.dvs_6axis, (int)need_dz);
			continue;
		}
		if (online && candidate->input.source == IA_CSS_BINARY_INPUT_MEMORY) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: %d && (%d == %d)\n",
				__LINE__, online, candidate->input.source,
				IA_CSS_BINARY_INPUT_MEMORY);
			continue;
		}
		if (!online && candidate->input.source == IA_CSS_BINARY_INPUT_SENSOR) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: !%d && (%d == %d)\n",
				__LINE__, online, candidate->input.source,
				IA_CSS_BINARY_INPUT_SENSOR);
			continue;
		}
		if (req_bin_out_info->res.width < candidate->output.min_width ||
		    req_bin_out_info->res.width > candidate->output.max_width) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: (%d > %d) || (%d < %d)\n",
				__LINE__,
				req_bin_out_info->padded_width,
				candidate->output.min_width,
				req_bin_out_info->padded_width,
				candidate->output.max_width);
			continue;
		}
		if (xcandidate->num_output_pins > 1 && /* in case we have a second output pin, */
		     req_vf_info) { /* and we need vf output. */
			if (req_vf_info->res.width > candidate->output.max_width) {
				ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
					"ia_css_binary_find() [%d] continue: (%d < %d)\n",
					__LINE__,
					req_vf_info->res.width,
					candidate->output.max_width);
				continue;
			}
		}
		if (req_in_info->padded_width > candidate->input.max_width) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: (%d > %d)\n",
				__LINE__, req_in_info->padded_width,
				candidate->input.max_width);
			continue;
		}
		if (!binary_supports_output_format(xcandidate, req_bin_out_info->format)) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: !%d\n",
				__LINE__,
				binary_supports_output_format(xcandidate, req_bin_out_info->format));
			continue;
		}
#ifdef ISP2401
		if (!binary_supports_input_format(xcandidate, descr->stream_format)) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
					    "ia_css_binary_find() [%d] continue: !%d\n",
					    __LINE__,
					    binary_supports_input_format(xcandidate, req_in_info->format));
			continue;
		}
#endif
		if (xcandidate->num_output_pins > 1 && /* in case we have a second output pin, */
		    req_vf_info                   && /* and we need vf output. */
						      /* check if the required vf format
							 is supported. */
		    !binary_supports_output_format(xcandidate, req_vf_info->format)) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: (%d > %d) && (%p != NULL) && !%d\n",
				__LINE__, xcandidate->num_output_pins, 1,
				req_vf_info,
				binary_supports_output_format(xcandidate, req_vf_info->format));
			continue;
		}

		/* Check if vf_veceven supports the requested vf format */
		if (xcandidate->num_output_pins == 1 &&
		    req_vf_info && candidate->enable.vf_veceven &&
		    !binary_supports_vf_format(xcandidate, req_vf_info->format)) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: (%d == %d) && (%p != NULL) && %d && !%d\n",
				__LINE__, xcandidate->num_output_pins, 1,
				req_vf_info, candidate->enable.vf_veceven,
				binary_supports_vf_format(xcandidate, req_vf_info->format));
			continue;
		}

		/* Check if vf_veceven supports the requested vf width */
		if (xcandidate->num_output_pins == 1 &&
		    req_vf_info && candidate->enable.vf_veceven) { /* and we need vf output. */
			if (req_vf_info->res.width > candidate->output.max_width) {
				ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
					"ia_css_binary_find() [%d] continue: (%d < %d)\n",
					__LINE__,
					req_vf_info->res.width,
					candidate->output.max_width);
				continue;
			}
		}

		if (!supports_bds_factor(candidate->bds.supported_bds_factors,
		    descr->required_bds_factor)) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: 0x%x & 0x%x)\n",
				__LINE__, candidate->bds.supported_bds_factors,
				descr->required_bds_factor);
			continue;
		}

		if (!candidate->enable.dpc && need_dpc) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: 0x%x & 0x%x)\n",
				__LINE__, candidate->enable.dpc,
				descr->enable_dpc);
			continue;
		}

		if (candidate->uds.use_bci && enable_capture_pp_bli) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: 0x%x & 0x%x)\n",
				__LINE__, candidate->uds.use_bci,
				descr->enable_capture_pp_bli);
			continue;
		}

#ifdef ISP2401
		if (candidate->enable.luma_only != enable_luma_only) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: %d != %d\n",
				__LINE__, candidate->enable.luma_only,
				descr->enable_luma_only);
			continue;
		}

		if(!candidate->enable.tnr && need_tnr) {
			ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
				"ia_css_binary_find() [%d] continue: !%d && %d\n",
				__LINE__, candidate->enable.tnr,
				descr->enable_tnr);
			continue;
		}

#endif
		/* reconfigure any variable properties of the binary */
		err = ia_css_binary_fill_info(xcandidate, online, two_ppc,
				       stream_format, req_in_info,
				       req_bds_out_info,
				       req_out_info, req_vf_info,
				       binary, &dvs_env,
				       descr->stream_config_left_padding,
				       false);

		if (err != IA_CSS_SUCCESS)
			break;
		binary_init_metrics(&binary->metrics, &binary->info->sp);
		break;
	}

	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
		"ia_css_binary_find() selected = %p, mode = %d ID = %d\n",
		xcandidate, xcandidate ? xcandidate->sp.pipeline.mode : 0, xcandidate ? xcandidate->sp.id : 0);

	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
		"ia_css_binary_find() leave: return_err=%d\n", err);

	return err;
}

unsigned
ia_css_binary_max_vf_width(void)
{
	/* This is (should be) true for IPU1 and IPU2 */
	/* For IPU3 (SkyCam) this pointer is guarenteed to be NULL simply because such a binary does not exist  */
	if (binary_infos[IA_CSS_BINARY_MODE_VF_PP])
		return binary_infos[IA_CSS_BINARY_MODE_VF_PP]->sp.output.max_width;
	return 0;
}

void
ia_css_binary_destroy_isp_parameters(struct ia_css_binary *binary)
{
	if (binary) {
		ia_css_isp_param_destroy_isp_parameters(&binary->mem_params,
							&binary->css_params);
	}
}

void
ia_css_binary_get_isp_binaries(struct ia_css_binary_xinfo **binaries,
	uint32_t *num_isp_binaries)
{
	assert(binaries != NULL);

	if (num_isp_binaries)
		*num_isp_binaries = 0;

	*binaries = all_binaries;
	if (all_binaries && num_isp_binaries) {
		/* -1 to account for sp binary which is not stored in all_binaries */
		if (sh_css_num_binaries > 0)
			*num_isp_binaries = sh_css_num_binaries - 1;
	}
}
{
	int mode;
	bool online;
	bool two_ppc;
	enum ia_css_stream_format stream_format;
	const struct ia_css_frame_info *req_in_info,
				       *req_bds_out_info,
				       *req_out_info[IA_CSS_BINARY_MAX_OUTPUT_PORTS],
				       *req_bin_out_info = NULL,
				       *req_vf_info;

	struct ia_css_binary_xinfo *xcandidate;
#ifndef ISP2401
	bool need_ds, need_dz, need_dvs, need_xnr, need_dpc;
#else
	bool need_ds, need_dz, need_dvs, need_xnr, need_dpc, need_tnr;
#endif
	bool striped;
	bool enable_yuv_ds;
	bool enable_high_speed;
	bool enable_dvs_6axis;
	bool enable_reduced_pipe;
	bool enable_capture_pp_bli;
#ifdef ISP2401
	bool enable_luma_only;
#endif
	enum ia_css_err err = IA_CSS_ERR_INTERNAL_ERROR;
	bool continuous;
	unsigned int isp_pipe_version;
	struct ia_css_resolution dvs_env, internal_res;
	unsigned int i;

	assert(descr != NULL);
	/* MW: used after an error check, may accept NULL, but doubtfull */
	assert(binary != NULL);

	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
		"ia_css_binary_find() enter: descr=%p, (mode=%d), binary=%p\n",
		descr, descr->mode,
		binary);

	mode = descr->mode;
	online = descr->online;
	two_ppc = descr->two_ppc;
	stream_format = descr->stream_format;
	req_in_info = descr->in_info;
	req_bds_out_info = descr->bds_out_info;
	for (i = 0; i < IA_CSS_BINARY_MAX_OUTPUT_PORTS; i++) {
		req_out_info[i] = descr->out_info[i];
		if (req_out_info[i] && (req_out_info[i]->res.width != 0))
			req_bin_out_info = req_out_info[i];
	}