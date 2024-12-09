{
	struct v4l2_event event = {0};

	event.type = V4L2_EVENT_ATOMISP_CSS_RESET;

	v4l2_event_queue(asd->subdev.devnode, &event);
}


static void print_csi_rx_errors(enum ia_css_csi2_port port,
				struct atomisp_device *isp)
{
	u32 infos = 0;

	atomisp_css_rx_get_irq_info(port, &infos);

	dev_err(isp->dev, "CSI Receiver port %d errors:\n", port);
	if (infos & CSS_RX_IRQ_INFO_BUFFER_OVERRUN)
		dev_err(isp->dev, "  buffer overrun");
	if (infos & CSS_RX_IRQ_INFO_ERR_SOT)
		dev_err(isp->dev, "  start-of-transmission error");
	if (infos & CSS_RX_IRQ_INFO_ERR_SOT_SYNC)
		dev_err(isp->dev, "  start-of-transmission sync error");
	if (infos & CSS_RX_IRQ_INFO_ERR_CONTROL)
		dev_err(isp->dev, "  control error");
	if (infos & CSS_RX_IRQ_INFO_ERR_ECC_DOUBLE)
		dev_err(isp->dev, "  2 or more ECC errors");
	if (infos & CSS_RX_IRQ_INFO_ERR_CRC)
		dev_err(isp->dev, "  CRC mismatch");
	if (infos & CSS_RX_IRQ_INFO_ERR_UNKNOWN_ID)
		dev_err(isp->dev, "  unknown error");
	if (infos & CSS_RX_IRQ_INFO_ERR_FRAME_SYNC)
		dev_err(isp->dev, "  frame sync error");
	if (infos & CSS_RX_IRQ_INFO_ERR_FRAME_DATA)
		dev_err(isp->dev, "  frame data error");
	if (infos & CSS_RX_IRQ_INFO_ERR_DATA_TIMEOUT)
		dev_err(isp->dev, "  data timeout");
	if (infos & CSS_RX_IRQ_INFO_ERR_UNKNOWN_ESC)
		dev_err(isp->dev, "  unknown escape command entry");
	if (infos & CSS_RX_IRQ_INFO_ERR_LINE_SYNC)
		dev_err(isp->dev, "  line sync error");
}
{
	u32 msg_ret;
	pci_read_config_dword(isp->pdev, PCI_INTERRUPT_CTRL, &msg_ret);
	msg_ret |= 1 << INTR_IIR;
	pci_write_config_dword(isp->pdev, PCI_INTERRUPT_CTRL, msg_ret);
}

static struct atomisp_sub_device *
__get_asd_from_port(struct atomisp_device *isp, mipi_port_ID_t port)
{
	int i;

	/* Check which isp subdev to send eof */
	for (i = 0; i < isp->num_of_streams; i++) {
		struct atomisp_sub_device *asd = &isp->asd[i];
		struct camera_mipi_info *mipi_info;

		mipi_info = atomisp_to_sensor_mipi_info(
				isp->inputs[asd->input_curr].camera);

		if (asd->streaming == ATOMISP_DEVICE_STREAMING_ENABLED &&
		    __get_mipi_port(isp, mipi_info->port) == port) {
			return asd;
		}
	}

	return NULL;
}
{
	struct atomisp_device *isp = (struct atomisp_device *)dev;
	struct atomisp_sub_device *asd;
	struct atomisp_css_event eof_event;
	unsigned int irq_infos = 0;
	unsigned long flags;
	unsigned int i;
	int err;

	spin_lock_irqsave(&isp->lock, flags);
	if (isp->sw_contex.power_state != ATOM_ISP_POWER_UP ||
	    isp->css_initialized == false) {
		spin_unlock_irqrestore(&isp->lock, flags);
		return IRQ_HANDLED;
	}
	    (irq_infos & CSS_IRQ_INFO_IF_ERROR)) {
		/* handle mipi receiver error */
		u32 rx_infos;
		enum ia_css_csi2_port port;

		for (port = IA_CSS_CSI2_PORT0; port <= IA_CSS_CSI2_PORT2;
		     port++) {
			print_csi_rx_errors(port, isp);
			atomisp_css_rx_get_irq_info(port, &rx_infos);
			atomisp_css_rx_clear_irq_info(port, rx_infos);
		}
	if (flag == 0) {
		*value = asd->params.fpn_en;
		return 0;
	}

	if (*value == 0) {
		asd->params.fpn_en = 0;
		return 0;
	}
	else if (field != V4L2_FIELD_NONE) {
		dev_err(isp->dev, "Wrong output field\n");
		return -EINVAL;
	}

	f->fmt.pix.field = field;
	f->fmt.pix.width = clamp_t(u32,
				   rounddown(width, (u32)ATOM_ISP_STEP_WIDTH),
				   ATOM_ISP_MIN_WIDTH, ATOM_ISP_MAX_WIDTH);
	f->fmt.pix.height = clamp_t(u32, rounddown(height,
				    (u32)ATOM_ISP_STEP_HEIGHT),
				    ATOM_ISP_MIN_HEIGHT, ATOM_ISP_MAX_HEIGHT);
	f->fmt.pix.bytesperline = (width * depth) >> 3;

	return 0;
}

mipi_port_ID_t __get_mipi_port(struct atomisp_device *isp,
				enum atomisp_camera_port port)
{
	switch (port) {
	case ATOMISP_CAMERA_PORT_PRIMARY:
		return MIPI_PORT0_ID;
	case ATOMISP_CAMERA_PORT_SECONDARY:
		return MIPI_PORT1_ID;
	case ATOMISP_CAMERA_PORT_TERTIARY:
		if (MIPI_PORT1_ID + 1 != N_MIPI_PORT_ID)
			return MIPI_PORT1_ID + 1;
		/* go through down for else case */
	default:
		dev_err(isp->dev, "unsupported port: %d\n", port);
		return MIPI_PORT0_ID;
	}
	if (!enable) {
		atomisp_css_enable_raw_binning(asd, false);
		atomisp_css_input_set_two_pixels_per_clock(asd, false);
	}

	if (isp->inputs[asd->input_curr].type != FILE_INPUT)
		atomisp_css_input_set_mode(asd, CSS_INPUT_MODE_SENSOR);

	return atomisp_update_run_mode(asd);
}

int configure_pp_input_nop(struct atomisp_sub_device *asd,
			   unsigned int width, unsigned int height)
{
	return 0;
}

int configure_output_nop(struct atomisp_sub_device *asd,
			 unsigned int width, unsigned int height,
			 unsigned int min_width,
			 enum atomisp_css_frame_format sh_fmt)
{
	return 0;
}

int get_frame_info_nop(struct atomisp_sub_device *asd,
		       struct atomisp_css_frame_info *finfo)
{
	return 0;
}

/*
 * Resets CSS parameters that depend on input resolution.
 *
 * Update params like CSS RAW binning, 2ppc mode and pp_input
 * which depend on input size, but are not automatically
 * handled in CSS when the input resolution is changed.
 */
static int css_input_resolution_changed(struct atomisp_sub_device *asd,
		struct v4l2_mbus_framefmt *ffmt)
{
	struct atomisp_metadata_buf *md_buf = NULL, *_md_buf;
	unsigned int i;

	dev_dbg(asd->isp->dev, "css_input_resolution_changed to %ux%u\n",
		ffmt->width, ffmt->height);

#if defined(ISP2401_NEW_INPUT_SYSTEM)
	atomisp_css_input_set_two_pixels_per_clock(asd, false);
#else
	atomisp_css_input_set_two_pixels_per_clock(asd, true);
#endif
	if (asd->continuous_mode->val) {
		/* Note for all checks: ffmt includes pad_w+pad_h */
		if (asd->run_mode->val == ATOMISP_RUN_MODE_VIDEO ||
		    (ffmt->width >= 2048 || ffmt->height >= 1536)) {
			/*
			 * For preview pipe, enable only if resolution
			 * is >= 3M for ISP2400.
			 */
			atomisp_css_enable_raw_binning(asd, true);
		}
	}
{
	struct atomisp_device *isp = asd->isp;

	/* if subdev type is SOC camera,we do not need to set DVS */
	if (isp->inputs[asd->input_curr].type == SOC_CAMERA)
		asd->params.video_dis_en = 0;

	if (asd->params.video_dis_en &&
	    asd->run_mode->val == ATOMISP_RUN_MODE_VIDEO) {
		/* envelope is 20% of the output resolution */
		/*
		 * dvs envelope cannot be round up.
		 * it would cause ISP timeout and color switch issue
		 */
		*dvs_env_w = rounddown(width / 5, ATOM_ISP_STEP_WIDTH);
		*dvs_env_h = rounddown(height / 5, ATOM_ISP_STEP_HEIGHT);
	}
		    ffmt->height < req_ffmt->height) {
			req_ffmt->height -= dvs_env_h;
			req_ffmt->width -= dvs_env_w;
			ffmt = req_ffmt;
			dev_warn(isp->dev,
			  "can not enable video dis due to sensor limitation.");
			asd->params.video_dis_en = 0;
		}
	}
	dev_dbg(isp->dev, "sensor width: %d, height: %d\n",
		ffmt->width, ffmt->height);
	vformat.which = V4L2_SUBDEV_FORMAT_ACTIVE;
	ret = v4l2_subdev_call(isp->inputs[asd->input_curr].camera, pad,
			       set_fmt, NULL, &vformat);
	if (ret)
		return ret;

	__atomisp_update_stream_env(asd, stream_index, stream_info);

	dev_dbg(isp->dev, "sensor width: %d, height: %d\n",
		ffmt->width, ffmt->height);

	if (ffmt->width < ATOM_ISP_STEP_WIDTH ||
	    ffmt->height < ATOM_ISP_STEP_HEIGHT)
			return -EINVAL;

	if (asd->params.video_dis_en &&
	    source_pad == ATOMISP_SUBDEV_PAD_SOURCE_VIDEO &&
	    (ffmt->width < req_ffmt->width || ffmt->height < req_ffmt->height)) {
		dev_warn(isp->dev,
			 "can not enable video dis due to sensor limitation.");
		asd->params.video_dis_en = 0;
	}

	atomisp_subdev_set_ffmt(&asd->subdev, fh.pad,
				V4L2_SUBDEV_FORMAT_ACTIVE,
				ATOMISP_SUBDEV_PAD_SINK, ffmt);

	return css_input_resolution_changed(asd, ffmt);
}

int atomisp_set_fmt(struct video_device *vdev, struct v4l2_format *f)
{
	struct atomisp_device *isp = video_get_drvdata(vdev);
	struct atomisp_video_pipe *pipe = atomisp_to_video_pipe(vdev);
	struct atomisp_sub_device *asd = pipe->asd;
	const struct atomisp_format_bridge *format_bridge;
	const struct atomisp_format_bridge *snr_format_bridge;
	struct atomisp_css_frame_info output_info, raw_output_info;
	struct v4l2_format snr_fmt = *f;
	struct v4l2_format backup_fmt = *f, s_fmt = *f;
	unsigned int dvs_env_w = 0, dvs_env_h = 0;
	unsigned int padding_w = pad_w, padding_h = pad_h;
	bool res_overflow = false, crop_needs_override = false;
	struct v4l2_mbus_framefmt isp_sink_fmt;
	struct v4l2_mbus_framefmt isp_source_fmt = {0};
	struct v4l2_rect isp_sink_crop;
	uint16_t source_pad = atomisp_subdev_source_pad(vdev);
	struct v4l2_subdev_fh fh;
	int ret;

	dev_dbg(isp->dev,
		"setting resolution %ux%u on pad %u for asd%d, bytesperline %u\n",
		f->fmt.pix.width, f->fmt.pix.height, source_pad,
		asd->index, f->fmt.pix.bytesperline);

	if (source_pad >= ATOMISP_SUBDEV_PADS_NUM)
		return -EINVAL;

	if (asd->streaming == ATOMISP_DEVICE_STREAMING_ENABLED) {
		dev_warn(isp->dev, "ISP does not support set format while at streaming!\n");
		return -EBUSY;
	}

	v4l2_fh_init(&fh.vfh, vdev);

	format_bridge = atomisp_get_format_bridge(f->fmt.pix.pixelformat);
	if (format_bridge == NULL)
		return -EINVAL;

	pipe->sh_fmt = format_bridge->sh_fmt;
	pipe->pix.pixelformat = f->fmt.pix.pixelformat;

	if (source_pad == ATOMISP_SUBDEV_PAD_SOURCE_VF ||
	    (source_pad == ATOMISP_SUBDEV_PAD_SOURCE_PREVIEW
		&& asd->run_mode->val == ATOMISP_RUN_MODE_VIDEO)) {
		if (asd->fmt_auto->val) {
			struct v4l2_rect *capture_comp;
			struct v4l2_rect r = {0};

			r.width = f->fmt.pix.width;
			r.height = f->fmt.pix.height;

			if (source_pad == ATOMISP_SUBDEV_PAD_SOURCE_PREVIEW)
				capture_comp = atomisp_subdev_get_rect(
					&asd->subdev, NULL,
					V4L2_SUBDEV_FORMAT_ACTIVE,
					ATOMISP_SUBDEV_PAD_SOURCE_VIDEO,
					V4L2_SEL_TGT_COMPOSE);
			else
				capture_comp = atomisp_subdev_get_rect(
					&asd->subdev, NULL,
					V4L2_SUBDEV_FORMAT_ACTIVE,
					ATOMISP_SUBDEV_PAD_SOURCE_CAPTURE,
					V4L2_SEL_TGT_COMPOSE);

			if (capture_comp->width < r.width
			    || capture_comp->height < r.height) {
				r.width = capture_comp->width;
				r.height = capture_comp->height;
			}
	    (ffmt->width < req_ffmt->width || ffmt->height < req_ffmt->height)) {
		dev_warn(isp->dev,
			 "can not enable video dis due to sensor limitation.");
		asd->params.video_dis_en = 0;
	}

	atomisp_subdev_set_ffmt(&asd->subdev, fh.pad,
				V4L2_SUBDEV_FORMAT_ACTIVE,
				ATOMISP_SUBDEV_PAD_SINK, ffmt);

	return css_input_resolution_changed(asd, ffmt);
}

int atomisp_set_fmt(struct video_device *vdev, struct v4l2_format *f)
{
	struct atomisp_device *isp = video_get_drvdata(vdev);
	struct atomisp_video_pipe *pipe = atomisp_to_video_pipe(vdev);
	struct atomisp_sub_device *asd = pipe->asd;
	const struct atomisp_format_bridge *format_bridge;
	const struct atomisp_format_bridge *snr_format_bridge;
	struct atomisp_css_frame_info output_info, raw_output_info;
	struct v4l2_format snr_fmt = *f;
	struct v4l2_format backup_fmt = *f, s_fmt = *f;
	unsigned int dvs_env_w = 0, dvs_env_h = 0;
	unsigned int padding_w = pad_w, padding_h = pad_h;
	bool res_overflow = false, crop_needs_override = false;
	struct v4l2_mbus_framefmt isp_sink_fmt;
	struct v4l2_mbus_framefmt isp_source_fmt = {0};
	struct v4l2_rect isp_sink_crop;
	uint16_t source_pad = atomisp_subdev_source_pad(vdev);
	struct v4l2_subdev_fh fh;
	int ret;

	dev_dbg(isp->dev,
		"setting resolution %ux%u on pad %u for asd%d, bytesperline %u\n",
		f->fmt.pix.width, f->fmt.pix.height, source_pad,
		asd->index, f->fmt.pix.bytesperline);

	if (source_pad >= ATOMISP_SUBDEV_PADS_NUM)
		return -EINVAL;

	if (asd->streaming == ATOMISP_DEVICE_STREAMING_ENABLED) {
		dev_warn(isp->dev, "ISP does not support set format while at streaming!\n");
		return -EBUSY;
	}

	v4l2_fh_init(&fh.vfh, vdev);

	format_bridge = atomisp_get_format_bridge(f->fmt.pix.pixelformat);
	if (format_bridge == NULL)
		return -EINVAL;

	pipe->sh_fmt = format_bridge->sh_fmt;
	pipe->pix.pixelformat = f->fmt.pix.pixelformat;

	if (source_pad == ATOMISP_SUBDEV_PAD_SOURCE_VF ||
	    (source_pad == ATOMISP_SUBDEV_PAD_SOURCE_PREVIEW
		&& asd->run_mode->val == ATOMISP_RUN_MODE_VIDEO)) {
		if (asd->fmt_auto->val) {
			struct v4l2_rect *capture_comp;
			struct v4l2_rect r = {0};

			r.width = f->fmt.pix.width;
			r.height = f->fmt.pix.height;

			if (source_pad == ATOMISP_SUBDEV_PAD_SOURCE_PREVIEW)
				capture_comp = atomisp_subdev_get_rect(
					&asd->subdev, NULL,
					V4L2_SUBDEV_FORMAT_ACTIVE,
					ATOMISP_SUBDEV_PAD_SOURCE_VIDEO,
					V4L2_SEL_TGT_COMPOSE);
			else
				capture_comp = atomisp_subdev_get_rect(
					&asd->subdev, NULL,
					V4L2_SUBDEV_FORMAT_ACTIVE,
					ATOMISP_SUBDEV_PAD_SOURCE_CAPTURE,
					V4L2_SEL_TGT_COMPOSE);

			if (capture_comp->width < r.width
			    || capture_comp->height < r.height) {
				r.width = capture_comp->width;
				r.height = capture_comp->height;
			}

			atomisp_subdev_set_selection(
				&asd->subdev, fh.pad,
				V4L2_SUBDEV_FORMAT_ACTIVE, source_pad,
				V4L2_SEL_TGT_COMPOSE, 0, &r);

			f->fmt.pix.width = r.width;
			f->fmt.pix.height = r.height;
		}
	if (!user_shading_table->enable) {
		atomisp_css_set_shading_table(asd, NULL);
		asd->params.sc_en = 0;
		return 0;
	}
		if (ret) {
			free_table = shading_table;
			ret = -EFAULT;
			goto out;
		}
	}
	shading_table->sensor_width = user_shading_table->sensor_width;
	shading_table->sensor_height = user_shading_table->sensor_height;
	shading_table->fraction_bits = user_shading_table->fraction_bits;

	free_table = asd->params.css_param.shading_table;
	asd->params.css_param.shading_table = shading_table;
	atomisp_css_set_shading_table(asd, shading_table);
	asd->params.sc_en = 1;

out:
	if (free_table != NULL)
		atomisp_css_shading_table_free(free_table);

	return ret;
}

/*Turn off ISP dphy */
int atomisp_ospm_dphy_down(struct atomisp_device *isp)
{
	unsigned long flags;
	u32 reg;

	dev_dbg(isp->dev, "%s\n", __func__);

	/* if ISP timeout, we can force powerdown */
	if (isp->isp_timeout)
		goto done;

	if (!atomisp_dev_users(isp))
		goto done;

	spin_lock_irqsave(&isp->lock, flags);
	isp->sw_contex.power_state = ATOM_ISP_POWER_DOWN;
	spin_unlock_irqrestore(&isp->lock, flags);
done:
	/*
	 * MRFLD IUNIT DPHY is located in an always-power-on island
	 * MRFLD HW design need all CSI ports are disabled before
	 * powering down the IUNIT.
	 */
	pci_read_config_dword(isp->pdev, MRFLD_PCI_CSI_CONTROL, &reg);
	reg |= MRFLD_ALL_CSI_PORTS_OFF_MASK;
	pci_write_config_dword(isp->pdev, MRFLD_PCI_CSI_CONTROL, reg);
	return 0;
}

/*Turn on ISP dphy */
int atomisp_ospm_dphy_up(struct atomisp_device *isp)
{
	unsigned long flags;
	dev_dbg(isp->dev, "%s\n", __func__);

	spin_lock_irqsave(&isp->lock, flags);
	isp->sw_contex.power_state = ATOM_ISP_POWER_UP;
	spin_unlock_irqrestore(&isp->lock, flags);

	return 0;
}


int atomisp_exif_makernote(struct atomisp_sub_device *asd,
			   struct atomisp_makernote_info *config)
{
	struct v4l2_control ctrl;
	struct atomisp_device *isp = asd->isp;

	ctrl.id = V4L2_CID_FOCAL_ABSOLUTE;
	if (v4l2_g_ctrl
	    (isp->inputs[asd->input_curr].camera->ctrl_handler, &ctrl)) {
		dev_warn(isp->dev, "failed to g_ctrl for focal length\n");
		return -EINVAL;
	} else {
		config->focal_length = ctrl.value;
	}

	ctrl.id = V4L2_CID_FNUMBER_ABSOLUTE;
	if (v4l2_g_ctrl
	    (isp->inputs[asd->input_curr].camera->ctrl_handler, &ctrl)) {
		dev_warn(isp->dev, "failed to g_ctrl for f-number\n");
		return -EINVAL;
	} else {
		config->f_number_curr = ctrl.value;
	}

	ctrl.id = V4L2_CID_FNUMBER_RANGE;
	if (v4l2_g_ctrl
	    (isp->inputs[asd->input_curr].camera->ctrl_handler, &ctrl)) {
		dev_warn(isp->dev, "failed to g_ctrl for f number range\n");
		return -EINVAL;
	} else {
		config->f_number_range = ctrl.value;
	}

	return 0;
}

int atomisp_offline_capture_configure(struct atomisp_sub_device *asd,
			      struct atomisp_cont_capture_conf *cvf_config)
{
	struct v4l2_ctrl *c;

	/*
	* In case of M10MO ZSL capture case, we need to issue a separate
	* capture request to M10MO which will output captured jpeg image
	*/
	c = v4l2_ctrl_find(
		asd->isp->inputs[asd->input_curr].camera->ctrl_handler,
		V4L2_CID_START_ZSL_CAPTURE);
	if (c) {
		int ret;
		dev_dbg(asd->isp->dev, "%s trigger ZSL capture request\n",
			__func__);
		/* TODO: use the cvf_config */
		ret = v4l2_ctrl_s_ctrl(c, 1);
		if (ret)
			return ret;

		return v4l2_ctrl_s_ctrl(c, 0);
	}

	asd->params.offline_parm = *cvf_config;

	if (asd->params.offline_parm.num_captures) {
		if (asd->streaming == ATOMISP_DEVICE_STREAMING_DISABLED) {
			unsigned int init_raw_num;

			if (asd->enable_raw_buffer_lock->val) {
				init_raw_num =
					ATOMISP_CSS2_NUM_OFFLINE_INIT_CONTINUOUS_FRAMES_LOCK_EN;
				if (asd->run_mode->val == ATOMISP_RUN_MODE_VIDEO &&
				    asd->params.video_dis_en)
					init_raw_num +=
					    ATOMISP_CSS2_NUM_DVS_FRAME_DELAY;
			} else {
				init_raw_num =
					ATOMISP_CSS2_NUM_OFFLINE_INIT_CONTINUOUS_FRAMES;
			}

			/* TODO: this can be removed once user-space
			 *       has been updated to use control API */
			asd->continuous_raw_buffer_size->val =
				max_t(int,
				      asd->continuous_raw_buffer_size->val,
				      asd->params.offline_parm.
				      num_captures + init_raw_num);
			asd->continuous_raw_buffer_size->val =
				min_t(int, ATOMISP_CONT_RAW_FRAMES,
				      asd->continuous_raw_buffer_size->val);
		}
		asd->continuous_mode->val = true;
	} else {
		asd->continuous_mode->val = false;
		__enable_continuous_mode(asd, false);
	}

	return 0;
}

/*
 * set auto exposure metering window to camera sensor
 */
int atomisp_s_ae_window(struct atomisp_sub_device *asd,
			struct atomisp_ae_window *arg)
{
	struct atomisp_device *isp = asd->isp;
	/* Coverity CID 298071 - initialzize struct */
	struct v4l2_subdev_selection sel = { 0 };

	sel.r.left = arg->x_left;
	sel.r.top = arg->y_top;
	sel.r.width = arg->x_right - arg->x_left + 1;
	sel.r.height = arg->y_bottom - arg->y_top + 1;

	if (v4l2_subdev_call(isp->inputs[asd->input_curr].camera,
			     pad, set_selection, NULL, &sel)) {
		dev_err(isp->dev, "failed to call sensor set_selection.\n");
		return -EINVAL;
	}

	return 0;
}

int atomisp_flash_enable(struct atomisp_sub_device *asd, int num_frames)
{
	struct atomisp_device *isp = asd->isp;

	if (num_frames < 0) {
		dev_dbg(isp->dev, "%s ERROR: num_frames: %d\n", __func__,
				num_frames);
		return -EINVAL;
	}
	/* a requested flash is still in progress. */
	if (num_frames && asd->params.flash_state != ATOMISP_FLASH_IDLE) {
		dev_dbg(isp->dev, "%s flash busy: %d frames left: %d\n",
				__func__, asd->params.flash_state,
				asd->params.num_flash_frames);
		return -EBUSY;
	}

	asd->params.num_flash_frames = num_frames;
	asd->params.flash_state = ATOMISP_FLASH_REQUESTED;
	return 0;
}

int atomisp_source_pad_to_stream_id(struct atomisp_sub_device *asd,
					   uint16_t source_pad)
{
	int stream_id;
	struct atomisp_device *isp = asd->isp;

	if (isp->inputs[asd->input_curr].camera_caps->
			sensor[asd->sensor_curr].stream_num == 1)
		return ATOMISP_INPUT_STREAM_GENERAL;

	switch (source_pad) {
	case ATOMISP_SUBDEV_PAD_SOURCE_CAPTURE:
		stream_id = ATOMISP_INPUT_STREAM_CAPTURE;
		break;
	case ATOMISP_SUBDEV_PAD_SOURCE_VF:
		stream_id = ATOMISP_INPUT_STREAM_POSTVIEW;
		break;
	case ATOMISP_SUBDEV_PAD_SOURCE_PREVIEW:
		stream_id = ATOMISP_INPUT_STREAM_PREVIEW;
		break;
	case ATOMISP_SUBDEV_PAD_SOURCE_VIDEO:
		stream_id = ATOMISP_INPUT_STREAM_VIDEO;
		break;
	default:
		stream_id = ATOMISP_INPUT_STREAM_GENERAL;
	}

	return stream_id;
}

bool atomisp_is_vf_pipe(struct atomisp_video_pipe *pipe)
{
	struct atomisp_sub_device *asd = pipe->asd;

	if (pipe == &asd->video_out_vf)
		return true;

	if (asd->run_mode->val == ATOMISP_RUN_MODE_VIDEO &&
	    pipe == &asd->video_out_preview)
		return true;

	return false;
}

static int __checking_exp_id(struct atomisp_sub_device *asd, int exp_id)
{
	struct atomisp_device *isp = asd->isp;

	if (!asd->enable_raw_buffer_lock->val) {
		dev_warn(isp->dev, "%s Raw Buffer Lock is disable.\n", __func__);
		return -EINVAL;
	}
	if (asd->streaming != ATOMISP_DEVICE_STREAMING_ENABLED) {
		dev_err(isp->dev, "%s streaming %d invalid exp_id %d.\n",
			__func__, exp_id, asd->streaming);
		return -EINVAL;
	}
	if ((exp_id > ATOMISP_MAX_EXP_ID) || (exp_id <= 0)) {
		dev_err(isp->dev, "%s exp_id %d invalid.\n", __func__, exp_id);
		return -EINVAL;
	}
	return 0;
}

void atomisp_init_raw_buffer_bitmap(struct atomisp_sub_device *asd)
{
	unsigned long flags;
	spin_lock_irqsave(&asd->raw_buffer_bitmap_lock, flags);
	memset(asd->raw_buffer_bitmap, 0, sizeof(asd->raw_buffer_bitmap));
	asd->raw_buffer_locked_count = 0;
	spin_unlock_irqrestore(&asd->raw_buffer_bitmap_lock, flags);
}

int atomisp_set_raw_buffer_bitmap(struct atomisp_sub_device *asd, int exp_id)
{
	int *bitmap, bit;
	unsigned long flags;

	if (__checking_exp_id(asd, exp_id))
		return -EINVAL;

	bitmap = asd->raw_buffer_bitmap + exp_id / 32;
	bit = exp_id % 32;
	spin_lock_irqsave(&asd->raw_buffer_bitmap_lock, flags);
	(*bitmap) |= (1 << bit);
	asd->raw_buffer_locked_count++;
	spin_unlock_irqrestore(&asd->raw_buffer_bitmap_lock, flags);

	dev_dbg(asd->isp->dev, "%s: exp_id %d,  raw_buffer_locked_count %d\n",
		__func__, exp_id, asd->raw_buffer_locked_count);

	/* Check if the raw buffer after next is still locked!!! */
	exp_id += 2;
	if (exp_id > ATOMISP_MAX_EXP_ID)
		exp_id -= ATOMISP_MAX_EXP_ID;
	bitmap = asd->raw_buffer_bitmap + exp_id / 32;
	bit = exp_id % 32;
	if ((*bitmap) & (1 << bit)) {
		int ret;

		/* WORKAROUND unlock the raw buffer compulsively */
		ret = atomisp_css_exp_id_unlock(asd, exp_id);
		if (ret) {
			dev_err(asd->isp->dev, "%s exp_id is wrapping back to %d but force unlock failed,, err %d.\n",
				__func__, exp_id, ret);
			return ret;
		}

		spin_lock_irqsave(&asd->raw_buffer_bitmap_lock, flags);
		(*bitmap) &= ~(1 << bit);
		asd->raw_buffer_locked_count--;
		spin_unlock_irqrestore(&asd->raw_buffer_bitmap_lock, flags);
		dev_warn(asd->isp->dev, "%s exp_id is wrapping back to %d but it is still locked so force unlock it, raw_buffer_locked_count %d\n",
			__func__, exp_id, asd->raw_buffer_locked_count);
	}
	return 0;
}

static int __is_raw_buffer_locked(struct atomisp_sub_device *asd, int exp_id)
{
	int *bitmap, bit;
	unsigned long flags;
	int ret;

	if (__checking_exp_id(asd, exp_id))
		return -EINVAL;

	bitmap = asd->raw_buffer_bitmap + exp_id / 32;
	bit = exp_id % 32;
	spin_lock_irqsave(&asd->raw_buffer_bitmap_lock, flags);
	ret = ((*bitmap) & (1 << bit));
	spin_unlock_irqrestore(&asd->raw_buffer_bitmap_lock, flags);
	return !ret;
}

static int __clear_raw_buffer_bitmap(struct atomisp_sub_device *asd, int exp_id)
{
	int *bitmap, bit;
	unsigned long flags;

	if (__is_raw_buffer_locked(asd, exp_id))
		return -EINVAL;

	bitmap = asd->raw_buffer_bitmap + exp_id / 32;
	bit = exp_id % 32;
	spin_lock_irqsave(&asd->raw_buffer_bitmap_lock, flags);
	(*bitmap) &= ~(1 << bit);
	asd->raw_buffer_locked_count--;
	spin_unlock_irqrestore(&asd->raw_buffer_bitmap_lock, flags);

	dev_dbg(asd->isp->dev, "%s: exp_id %d,  raw_buffer_locked_count %d\n",
		__func__, exp_id, asd->raw_buffer_locked_count);
	return 0;
}

int atomisp_exp_id_capture(struct atomisp_sub_device *asd, int *exp_id)
{
	struct atomisp_device *isp = asd->isp;
	int value = *exp_id;
	int ret;

	ret = __is_raw_buffer_locked(asd, value);
	if (ret) {
		dev_err(isp->dev, "%s exp_id %d invalid %d.\n", __func__, value, ret);
		return -EINVAL;
	}

	dev_dbg(isp->dev, "%s exp_id %d\n", __func__, value);
	ret = atomisp_css_exp_id_capture(asd, value);
	if (ret) {
		dev_err(isp->dev, "%s exp_id %d failed.\n", __func__, value);
		return -EIO;
	}
	return 0;
}

int atomisp_exp_id_unlock(struct atomisp_sub_device *asd, int *exp_id)
{
	struct atomisp_device *isp = asd->isp;
	int value = *exp_id;
	int ret;

	ret = __clear_raw_buffer_bitmap(asd, value);
	if (ret) {
		dev_err(isp->dev, "%s exp_id %d invalid %d.\n", __func__, value, ret);
		return -EINVAL;
	}

	dev_dbg(isp->dev, "%s exp_id %d\n", __func__, value);
	ret = atomisp_css_exp_id_unlock(asd, value);
	if (ret)
		dev_err(isp->dev, "%s exp_id %d failed, err %d.\n",
			__func__, value, ret);

	return ret;
}

int atomisp_enable_dz_capt_pipe(struct atomisp_sub_device *asd,
					   unsigned int *enable)
{
	bool value;

	if (enable == NULL)
		return -EINVAL;

	value = *enable > 0 ? true : false;

	atomisp_en_dz_capt_pipe(asd, value);

	return 0;
}

int atomisp_inject_a_fake_event(struct atomisp_sub_device *asd, int *event)
{
	if (!event || asd->streaming != ATOMISP_DEVICE_STREAMING_ENABLED)
		return -EINVAL;

	dev_dbg(asd->isp->dev, "%s: trying to inject a fake event 0x%x\n",
		__func__, *event);

	switch (*event) {
	case V4L2_EVENT_FRAME_SYNC:
		atomisp_sof_event(asd);
		break;
	case V4L2_EVENT_FRAME_END:
		atomisp_eof_event(asd, 0);
		break;
	case V4L2_EVENT_ATOMISP_3A_STATS_READY:
		atomisp_3a_stats_ready_event(asd, 0);
		break;
	case V4L2_EVENT_ATOMISP_METADATA_READY:
		atomisp_metadata_ready_event(asd, 0);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

int atomisp_get_pipe_id(struct atomisp_video_pipe *pipe)
{
	struct atomisp_sub_device *asd = pipe->asd;

	if (ATOMISP_USE_YUVPP(asd))
		return CSS_PIPE_ID_YUVPP;
	else if (asd->vfpp->val == ATOMISP_VFPP_DISABLE_SCALER)
		return CSS_PIPE_ID_VIDEO;
	else if (asd->vfpp->val == ATOMISP_VFPP_DISABLE_LOWLAT)
		return CSS_PIPE_ID_CAPTURE;
	else if (pipe == &asd->video_out_video_capture)
		return CSS_PIPE_ID_VIDEO;
	else if (pipe == &asd->video_out_vf)
		return CSS_PIPE_ID_CAPTURE;
	else if (pipe == &asd->video_out_preview) {
		if (asd->run_mode->val == ATOMISP_RUN_MODE_VIDEO)
			return CSS_PIPE_ID_VIDEO;
		else
			return CSS_PIPE_ID_PREVIEW;
	} else if (pipe == &asd->video_out_capture) {
		if (asd->copy_mode)
			return IA_CSS_PIPE_ID_COPY;
		else
			return CSS_PIPE_ID_CAPTURE;
	}

	/* fail through */
	dev_warn(asd->isp->dev, "%s failed to find proper pipe\n",
		__func__);
	return CSS_PIPE_ID_CAPTURE;
}

int atomisp_get_invalid_frame_num(struct video_device *vdev,
					int *invalid_frame_num)
{
	struct atomisp_video_pipe *pipe = atomisp_to_video_pipe(vdev);
	struct atomisp_sub_device *asd = pipe->asd;
	enum atomisp_css_pipe_id pipe_id;
	struct ia_css_pipe_info p_info;
	int ret;

	if (asd->isp->inputs[asd->input_curr].camera_caps->
		sensor[asd->sensor_curr].stream_num > 1) {
		/* External ISP */
		*invalid_frame_num = 0;
		return 0;
	}

	pipe_id = atomisp_get_pipe_id(pipe);
	if (!asd->stream_env[ATOMISP_INPUT_STREAM_GENERAL].pipes[pipe_id]) {
		dev_warn(asd->isp->dev, "%s pipe %d has not been created yet, do SET_FMT first!\n",
			__func__, pipe_id);
		return -EINVAL;
	}

	ret = ia_css_pipe_get_info(
		asd->stream_env[ATOMISP_INPUT_STREAM_GENERAL]
		.pipes[pipe_id], &p_info);
	if (ret == IA_CSS_SUCCESS) {
		*invalid_frame_num = p_info.num_invalid_frames;
		return 0;
	} else {
		dev_warn(asd->isp->dev, "%s get pipe infor failed %d\n",
			 __func__, ret);
		return -EINVAL;
	}
}
	switch (*event) {
	case V4L2_EVENT_FRAME_SYNC:
		atomisp_sof_event(asd);
		break;
	case V4L2_EVENT_FRAME_END:
		atomisp_eof_event(asd, 0);
		break;
	case V4L2_EVENT_ATOMISP_3A_STATS_READY:
		atomisp_3a_stats_ready_event(asd, 0);
		break;
	case V4L2_EVENT_ATOMISP_METADATA_READY:
		atomisp_metadata_ready_event(asd, 0);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

int atomisp_get_pipe_id(struct atomisp_video_pipe *pipe)
{
	struct atomisp_sub_device *asd = pipe->asd;

	if (ATOMISP_USE_YUVPP(asd))
		return CSS_PIPE_ID_YUVPP;
	else if (asd->vfpp->val == ATOMISP_VFPP_DISABLE_SCALER)
		return CSS_PIPE_ID_VIDEO;
	else if (asd->vfpp->val == ATOMISP_VFPP_DISABLE_LOWLAT)
		return CSS_PIPE_ID_CAPTURE;
	else if (pipe == &asd->video_out_video_capture)
		return CSS_PIPE_ID_VIDEO;
	else if (pipe == &asd->video_out_vf)
		return CSS_PIPE_ID_CAPTURE;
	else if (pipe == &asd->video_out_preview) {
		if (asd->run_mode->val == ATOMISP_RUN_MODE_VIDEO)
			return CSS_PIPE_ID_VIDEO;
		else
			return CSS_PIPE_ID_PREVIEW;
	} else if (pipe == &asd->video_out_capture) {
		if (asd->copy_mode)
			return IA_CSS_PIPE_ID_COPY;
		else
			return CSS_PIPE_ID_CAPTURE;
	}

	/* fail through */
	dev_warn(asd->isp->dev, "%s failed to find proper pipe\n",
		__func__);
	return CSS_PIPE_ID_CAPTURE;
}