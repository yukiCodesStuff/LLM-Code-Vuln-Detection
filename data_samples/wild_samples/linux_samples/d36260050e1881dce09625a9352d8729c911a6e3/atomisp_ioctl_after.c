	if (ret) {
		dev_err(isp->dev, "depth mode slave sensor %s stream-on failed.\n",
			isp->inputs[slave].camera->name);
		v4l2_subdev_call(isp->inputs[master].camera, video, s_stream, 0);

		return -EINVAL;
	}

	return 0;
}

/* FIXME! */
#ifndef ISP2401
static void __wdt_on_master_slave_sensor(struct atomisp_device *isp,
					 unsigned int wdt_duration)
#else
static void __wdt_on_master_slave_sensor(struct atomisp_video_pipe *pipe,
					 unsigned int wdt_duration,
					 bool enable)
#endif
{
#ifndef ISP2401
	if (atomisp_buffers_queued(&isp->asd[0]))
		atomisp_wdt_refresh(&isp->asd[0], wdt_duration);
	if (atomisp_buffers_queued(&isp->asd[1]))
		atomisp_wdt_refresh(&isp->asd[1], wdt_duration);
#else
	static struct atomisp_video_pipe *pipe0;

	if (enable) {
		if (atomisp_buffers_queued_pipe(pipe0))
			atomisp_wdt_refresh_pipe(pipe0, wdt_duration);
		if (atomisp_buffers_queued_pipe(pipe))
			atomisp_wdt_refresh_pipe(pipe, wdt_duration);
	} else {
		pipe0 = pipe;
	}
#endif
}

static void atomisp_pause_buffer_event(struct atomisp_device *isp)
{
	struct v4l2_event event = {0};
	int i;

	event.type = V4L2_EVENT_ATOMISP_PAUSE_BUFFER;

	for (i = 0; i < isp->num_of_streams; i++) {
		int sensor_index = isp->asd[i].input_curr;
		if (isp->inputs[sensor_index].camera_caps->
				sensor[isp->asd[i].sensor_curr].is_slave) {
			v4l2_event_queue(isp->asd[i].subdev.devnode, &event);
			break;
		}
	}
}

/* Input system HW workaround */
/* Input system address translation corrupts burst during */
/* invalidate. SW workaround for this is to set burst length */
/* manually to 128 in case of 13MPx snapshot and to 1 otherwise. */
static void atomisp_dma_burst_len_cfg(struct atomisp_sub_device *asd)
{

	struct v4l2_mbus_framefmt *sink;
	sink = atomisp_subdev_get_ffmt(&asd->subdev, NULL,
				       V4L2_SUBDEV_FORMAT_ACTIVE,
				       ATOMISP_SUBDEV_PAD_SINK);

	if (sink->width * sink->height >= 4096*3072)
		atomisp_store_uint32(DMA_BURST_SIZE_REG, 0x7F);
	else
		atomisp_store_uint32(DMA_BURST_SIZE_REG, 0x00);
}

/*
 * This ioctl start the capture during streaming I/O.
 */
static int atomisp_streamon(struct file *file, void *fh,
	enum v4l2_buf_type type)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_video_pipe *pipe = atomisp_to_video_pipe(vdev);
	struct atomisp_sub_device *asd = pipe->asd;
	struct atomisp_device *isp = video_get_drvdata(vdev);
	enum atomisp_css_pipe_id css_pipe_id;
	unsigned int sensor_start_stream;
	unsigned int wdt_duration = ATOMISP_ISP_TIMEOUT_DURATION;
	int ret = 0;
	unsigned long irqflags;

	dev_dbg(isp->dev, "Start stream on pad %d for asd%d\n",
		atomisp_subdev_source_pad(vdev), asd->index);

	if (type != V4L2_BUF_TYPE_VIDEO_CAPTURE) {
		dev_dbg(isp->dev, "unsupported v4l2 buf type\n");
		return -EINVAL;
	}

	rt_mutex_lock(&isp->mutex);
	if (isp->isp_fatal_error) {
		ret = -EIO;
		goto out;
	}

	if (asd->streaming == ATOMISP_DEVICE_STREAMING_STOPPING) {
		ret = -EBUSY;
		goto out;
	}

	if (pipe->capq.streaming)
		goto out;

	/* Input system HW workaround */
	atomisp_dma_burst_len_cfg(asd);

	/*
	 * The number of streaming video nodes is based on which
	 * binary is going to be run.
	 */
	sensor_start_stream = atomisp_sensor_start_stream(asd);

	spin_lock_irqsave(&pipe->irq_lock, irqflags);
	if (list_empty(&(pipe->capq.stream))) {
		spin_unlock_irqrestore(&pipe->irq_lock, irqflags);
		dev_dbg(isp->dev, "no buffer in the queue\n");
		ret = -EINVAL;
		goto out;
	}
	spin_unlock_irqrestore(&pipe->irq_lock, irqflags);

	ret = videobuf_streamon(&pipe->capq);
	if (ret)
		goto out;

	/* Reset pending capture request count. */
	asd->pending_capture_request = 0;
#ifdef ISP2401
	asd->re_trigger_capture = false;
#endif

	if ((atomisp_subdev_streaming_count(asd) > sensor_start_stream) &&
	    (!isp->inputs[asd->input_curr].camera_caps->multi_stream_ctrl)) {
		/* trigger still capture */
		if (asd->continuous_mode->val &&
		    atomisp_subdev_source_pad(vdev)
		    == ATOMISP_SUBDEV_PAD_SOURCE_CAPTURE) {
			if (asd->run_mode->val == ATOMISP_RUN_MODE_VIDEO)
				dev_dbg(isp->dev, "SDV last video raw buffer id: %u\n",
					asd->latest_preview_exp_id);
			else
				dev_dbg(isp->dev, "ZSL last preview raw buffer id: %u\n",
					asd->latest_preview_exp_id);

			if (asd->delayed_init == ATOMISP_DELAYED_INIT_QUEUED) {
				flush_work(&asd->delayed_init_work);
				rt_mutex_unlock(&isp->mutex);
				if (wait_for_completion_interruptible(
						&asd->init_done) != 0)
					return -ERESTARTSYS;
				rt_mutex_lock(&isp->mutex);
			}

			/* handle per_frame_setting parameter and buffers */
			atomisp_handle_parameter_and_buffer(pipe);

			/*
			 * only ZSL/SDV capture request will be here, raise
			 * the ISP freq to the highest possible to minimize
			 * the S2S latency.
			 */
			atomisp_freq_scaling(isp, ATOMISP_DFS_MODE_MAX, false);
			/*
			 * When asd->enable_raw_buffer_lock->val is true,
			 * An extra IOCTL is needed to call
			 * atomisp_css_exp_id_capture and trigger real capture
			 */
			if (!asd->enable_raw_buffer_lock->val) {
				ret = atomisp_css_offline_capture_configure(asd,
					asd->params.offline_parm.num_captures,
					asd->params.offline_parm.skip_frames,
					asd->params.offline_parm.offset);
				if (ret) {
					ret = -EINVAL;
					goto out;
				}
				if (asd->depth_mode->val)
					atomisp_pause_buffer_event(isp);
			}
		}
		atomisp_qbuffers_to_css(asd);
		goto out;
	}

	if (asd->streaming == ATOMISP_DEVICE_STREAMING_ENABLED) {
		atomisp_qbuffers_to_css(asd);
		goto start_sensor;
	}

	css_pipe_id = atomisp_get_css_pipe_id(asd);

	ret = atomisp_acc_load_extensions(asd);
	if (ret < 0) {
		dev_err(isp->dev, "acc extension failed to load\n");
		goto out;
	}

	if (asd->params.css_update_params_needed) {
		atomisp_apply_css_parameters(asd, &asd->params.css_param);
		if (asd->params.css_param.update_flag.dz_config)
			atomisp_css_set_dz_config(asd,
				&asd->params.css_param.dz_config);
		atomisp_css_update_isp_params(asd);
		asd->params.css_update_params_needed = false;
		memset(&asd->params.css_param.update_flag, 0,
		       sizeof(struct atomisp_parameters));
	}
	asd->params.dvs_6axis = NULL;

	ret = atomisp_css_start(asd, css_pipe_id, false);
	if (ret)
		goto out;

	asd->streaming = ATOMISP_DEVICE_STREAMING_ENABLED;
	atomic_set(&asd->sof_count, -1);
	atomic_set(&asd->sequence, -1);
	atomic_set(&asd->sequence_temp, -1);
	if (isp->sw_contex.file_input)
		wdt_duration = ATOMISP_ISP_FILE_TIMEOUT_DURATION;

	asd->params.dis_proj_data_valid = false;
	asd->latest_preview_exp_id = 0;
	asd->postview_exp_id = 1;
	asd->preview_exp_id = 1;

	/* handle per_frame_setting parameter and buffers */
	atomisp_handle_parameter_and_buffer(pipe);

	atomisp_qbuffers_to_css(asd);

	/* Only start sensor when the last streaming instance started */
	if (atomisp_subdev_streaming_count(asd) < sensor_start_stream)
		goto out;

start_sensor:
	if (isp->flash) {
		asd->params.num_flash_frames = 0;
		asd->params.flash_state = ATOMISP_FLASH_IDLE;
		atomisp_setup_flash(asd);
	}

	if (!isp->sw_contex.file_input) {
		atomisp_css_irq_enable(isp, CSS_IRQ_INFO_CSS_RECEIVER_SOF,
				atomisp_css_valid_sof(isp));
		atomisp_csi2_configure(asd);
		/*
		 * set freq to max when streaming count > 1 which indicate
		 * dual camera would run
		*/
		if (atomisp_streaming_count(isp) > 1) {
			if (atomisp_freq_scaling(isp,
				ATOMISP_DFS_MODE_MAX, false) < 0)
				dev_dbg(isp->dev, "dfs failed!\n");
		} else {
			if (atomisp_freq_scaling(isp,
				ATOMISP_DFS_MODE_AUTO, false) < 0)
				dev_dbg(isp->dev, "dfs failed!\n");
		}
	} else {
		if (atomisp_freq_scaling(isp, ATOMISP_DFS_MODE_MAX, false) < 0)
			dev_dbg(isp->dev, "dfs failed!\n");
	}

	if (asd->depth_mode->val && atomisp_streaming_count(isp) ==
			ATOMISP_DEPTH_SENSOR_STREAMON_COUNT) {
		ret = atomisp_stream_on_master_slave_sensor(isp, false);
		if (ret) {
			dev_err(isp->dev, "master slave sensor stream on failed!\n");
			goto out;
		}
#ifndef ISP2401
		__wdt_on_master_slave_sensor(isp, wdt_duration);
#else
		__wdt_on_master_slave_sensor(pipe, wdt_duration, true);
#endif
		goto start_delay_wq;
	} else if (asd->depth_mode->val && (atomisp_streaming_count(isp) <
		   ATOMISP_DEPTH_SENSOR_STREAMON_COUNT)) {
#ifdef ISP2401
		__wdt_on_master_slave_sensor(pipe, wdt_duration, false);
#endif
		goto start_delay_wq;
	}

	/* Enable the CSI interface on ANN B0/K0 */
	if (isp->media_dev.hw_revision >= ((ATOMISP_HW_REVISION_ISP2401 <<
	    ATOMISP_HW_REVISION_SHIFT) | ATOMISP_HW_STEPPING_B0)) {
		pci_write_config_word(isp->pdev, MRFLD_PCI_CSI_CONTROL,
				      isp->saved_regs.csi_control |
				      MRFLD_PCI_CSI_CONTROL_CSI_READY);
	}

	/* stream on the sensor */
	ret = v4l2_subdev_call(isp->inputs[asd->input_curr].camera,
			       video, s_stream, 1);
	if (ret) {
		asd->streaming = ATOMISP_DEVICE_STREAMING_DISABLED;
		ret = -EINVAL;
		goto out;
	}

#ifndef ISP2401
	if (atomisp_buffers_queued(asd))
		atomisp_wdt_refresh(asd, wdt_duration);
#else
	if (atomisp_buffers_queued_pipe(pipe))
		atomisp_wdt_refresh_pipe(pipe, wdt_duration);
#endif

start_delay_wq:
	if (asd->continuous_mode->val) {
		struct v4l2_mbus_framefmt *sink;

		sink = atomisp_subdev_get_ffmt(&asd->subdev, NULL,
				       V4L2_SUBDEV_FORMAT_ACTIVE,
				       ATOMISP_SUBDEV_PAD_SINK);

		reinit_completion(&asd->init_done);
		asd->delayed_init = ATOMISP_DELAYED_INIT_QUEUED;
		queue_work(asd->delayed_init_workq, &asd->delayed_init_work);
		atomisp_css_set_cont_prev_start_time(isp,
				ATOMISP_CALC_CSS_PREV_OVERLAP(sink->height));
	} else {
		asd->delayed_init = ATOMISP_DELAYED_INIT_NOT_QUEUED;
	}
out:
	rt_mutex_unlock(&isp->mutex);
	return ret;
}

int __atomisp_streamoff(struct file *file, void *fh, enum v4l2_buf_type type)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_device *isp = video_get_drvdata(vdev);
	struct atomisp_video_pipe *pipe = atomisp_to_video_pipe(vdev);
	struct atomisp_sub_device *asd = pipe->asd;
	struct atomisp_video_pipe *capture_pipe = NULL;
	struct atomisp_video_pipe *vf_pipe = NULL;
	struct atomisp_video_pipe *preview_pipe = NULL;
	struct atomisp_video_pipe *video_pipe = NULL;
	struct videobuf_buffer *vb, *_vb;
	enum atomisp_css_pipe_id css_pipe_id;
	int ret;
	unsigned long flags;
	bool first_streamoff = false;

	dev_dbg(isp->dev, "Stop stream on pad %d for asd%d\n",
		atomisp_subdev_source_pad(vdev), asd->index);

	BUG_ON(!rt_mutex_is_locked(&isp->mutex));
	BUG_ON(!mutex_is_locked(&isp->streamoff_mutex));

	if (type != V4L2_BUF_TYPE_VIDEO_CAPTURE) {
		dev_dbg(isp->dev, "unsupported v4l2 buf type\n");
		return -EINVAL;
	}

	/*
	 * do only videobuf_streamoff for capture & vf pipes in
	 * case of continuous capture
	 */
	if ((asd->continuous_mode->val ||
	    isp->inputs[asd->input_curr].camera_caps->multi_stream_ctrl) &&
	    atomisp_subdev_source_pad(vdev) !=
		ATOMISP_SUBDEV_PAD_SOURCE_PREVIEW &&
	    atomisp_subdev_source_pad(vdev) !=
		ATOMISP_SUBDEV_PAD_SOURCE_VIDEO) {

		if (isp->inputs[asd->input_curr].camera_caps->multi_stream_ctrl) {
			v4l2_subdev_call(isp->inputs[asd->input_curr].camera,
				video, s_stream, 0);
		} else if (atomisp_subdev_source_pad(vdev)
		    == ATOMISP_SUBDEV_PAD_SOURCE_CAPTURE) {
			/* stop continuous still capture if needed */
			if (asd->params.offline_parm.num_captures == -1)
				atomisp_css_offline_capture_configure(asd,
						0, 0, 0);
			atomisp_freq_scaling(isp, ATOMISP_DFS_MODE_AUTO, false);
		}
		/*
		 * Currently there is no way to flush buffers queued to css.
		 * When doing videobuf_streamoff, active buffers will be
		 * marked as VIDEOBUF_NEEDS_INIT. HAL will be able to use
		 * these buffers again, and these buffers might be queued to
		 * css more than once! Warn here, if HAL has not dequeued all
		 * buffers back before calling streamoff.
		 */
		if (pipe->buffers_in_css != 0) {
			WARN(1, "%s: buffers of vdev %s still in CSS!\n",
			     __func__, pipe->vdev.name);

			/*
			 * Buffers remained in css maybe dequeued out in the
			 * next stream on, while this will causes serious
			 * issues as buffers already get invalid after
			 * previous stream off.
			 *
			 * No way to flush buffers but to reset the whole css
			 */
			dev_warn(isp->dev, "Reset CSS to clean up css buffers.\n");
			atomisp_css_flush(isp);
		}

		return videobuf_streamoff(&pipe->capq);
	}

	if (!pipe->capq.streaming)
		return 0;

	spin_lock_irqsave(&isp->lock, flags);
	if (asd->streaming == ATOMISP_DEVICE_STREAMING_ENABLED) {
		asd->streaming = ATOMISP_DEVICE_STREAMING_STOPPING;
		first_streamoff = true;
	}
	spin_unlock_irqrestore(&isp->lock, flags);

	if (first_streamoff) {
		/* if other streams are running, should not disable watch dog */
		rt_mutex_unlock(&isp->mutex);
		atomisp_wdt_stop(asd, true);

		/*
		 * must stop sending pixels into GP_FIFO before stop
		 * the pipeline.
		 */
		if (isp->sw_contex.file_input)
			v4l2_subdev_call(isp->inputs[asd->input_curr].camera,
					video, s_stream, 0);

		rt_mutex_lock(&isp->mutex);
		atomisp_acc_unload_extensions(asd);
	}

	spin_lock_irqsave(&isp->lock, flags);
	if (atomisp_subdev_streaming_count(asd) == 1)
		asd->streaming = ATOMISP_DEVICE_STREAMING_DISABLED;
	spin_unlock_irqrestore(&isp->lock, flags);

	if (!first_streamoff) {
		ret = videobuf_streamoff(&pipe->capq);
		if (ret)
			return ret;
		goto stopsensor;
	}

	atomisp_clear_css_buffer_counters(asd);

	if (!isp->sw_contex.file_input)
		atomisp_css_irq_enable(isp, CSS_IRQ_INFO_CSS_RECEIVER_SOF,
					false);

	if (asd->delayed_init == ATOMISP_DELAYED_INIT_QUEUED) {
		cancel_work_sync(&asd->delayed_init_work);
		asd->delayed_init = ATOMISP_DELAYED_INIT_NOT_QUEUED;
	}
	if (first_streamoff) {
		css_pipe_id = atomisp_get_css_pipe_id(asd);
		ret = atomisp_css_stop(asd, css_pipe_id, false);
	}
	/* cancel work queue*/
	if (asd->video_out_capture.users) {
		capture_pipe = &asd->video_out_capture;
		wake_up_interruptible(&capture_pipe->capq.wait);
	}
	if (asd->video_out_vf.users) {
		vf_pipe = &asd->video_out_vf;
		wake_up_interruptible(&vf_pipe->capq.wait);
	}
	if (asd->video_out_preview.users) {
		preview_pipe = &asd->video_out_preview;
		wake_up_interruptible(&preview_pipe->capq.wait);
	}
	if (asd->video_out_video_capture.users) {
		video_pipe = &asd->video_out_video_capture;
		wake_up_interruptible(&video_pipe->capq.wait);
	}
	ret = videobuf_streamoff(&pipe->capq);
	if (ret)
		return ret;

	/* cleanup css here */
	/* no need for this, as ISP will be reset anyway */
	/*atomisp_flush_bufs_in_css(isp);*/

	spin_lock_irqsave(&pipe->irq_lock, flags);
	list_for_each_entry_safe(vb, _vb, &pipe->activeq, queue) {
		vb->state = VIDEOBUF_PREPARED;
		list_del(&vb->queue);
	}
	list_for_each_entry_safe(vb, _vb, &pipe->buffers_waiting_for_param, queue) {
		vb->state = VIDEOBUF_PREPARED;
		list_del(&vb->queue);
		pipe->frame_request_config_id[vb->i] = 0;
	}
	spin_unlock_irqrestore(&pipe->irq_lock, flags);

	atomisp_subdev_cleanup_pending_events(asd);
stopsensor:
	if (atomisp_subdev_streaming_count(asd) + 1
	    != atomisp_sensor_start_stream(asd))
		return 0;

	if (!isp->sw_contex.file_input)
		ret = v4l2_subdev_call(isp->inputs[asd->input_curr].camera,
				       video, s_stream, 0);

	if (isp->flash) {
		asd->params.num_flash_frames = 0;
		asd->params.flash_state = ATOMISP_FLASH_IDLE;
	}

	/* if other streams are running, isp should not be powered off */
	if (atomisp_streaming_count(isp)) {
		atomisp_css_flush(isp);
		return 0;
	}

	/* Disable the CSI interface on ANN B0/K0 */
	if (isp->media_dev.hw_revision >= ((ATOMISP_HW_REVISION_ISP2401 <<
	    ATOMISP_HW_REVISION_SHIFT) | ATOMISP_HW_STEPPING_B0)) {
		pci_write_config_word(isp->pdev, MRFLD_PCI_CSI_CONTROL,
				      isp->saved_regs.csi_control &
				      ~MRFLD_PCI_CSI_CONTROL_CSI_READY);
	}

	if (atomisp_freq_scaling(isp, ATOMISP_DFS_MODE_LOW, false))
		dev_warn(isp->dev, "DFS failed.\n");
	/*
	 * ISP work around, need to reset isp
	 * Is it correct time to reset ISP when first node does streamoff?
	 */
	if (isp->sw_contex.power_state == ATOM_ISP_POWER_UP) {
		unsigned int i;
		bool recreate_streams[MAX_STREAM_NUM] = {0};
		if (isp->isp_timeout)
			dev_err(isp->dev, "%s: Resetting with WA activated",
				__func__);
		/*
		 * It is possible that the other asd stream is in the stage
		 * that v4l2_setfmt is just get called on it, which will
		 * create css stream on that stream. But at this point, there
		 * is no way to destroy the css stream created on that stream.
		 *
		 * So force stream destroy here.
		 */
		for (i = 0; i < isp->num_of_streams; i++) {
			if (isp->asd[i].stream_prepared) {
				atomisp_destroy_pipes_stream_force(&isp->
						asd[i]);
				recreate_streams[i] = true;
			}
		}

		/* disable  PUNIT/ISP acknowlede/handshake - SRSE=3 */
		pci_write_config_dword(isp->pdev, PCI_I_CONTROL, isp->saved_regs.i_control |
				MRFLD_PCI_I_CONTROL_SRSE_RESET_MASK);
		dev_err(isp->dev, "atomisp_reset");
		atomisp_reset(isp);
		for (i = 0; i < isp->num_of_streams; i++) {
			if (recreate_streams[i])
				atomisp_create_pipes_stream(&isp->asd[i]);
		}
		isp->isp_timeout = false;
	}
	return ret;
}

static int atomisp_streamoff(struct file *file, void *fh,
			     enum v4l2_buf_type type)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_device *isp = video_get_drvdata(vdev);
	int rval;

	mutex_lock(&isp->streamoff_mutex);
	rt_mutex_lock(&isp->mutex);
	rval = __atomisp_streamoff(file, fh, type);
	rt_mutex_unlock(&isp->mutex);
	mutex_unlock(&isp->streamoff_mutex);

	return rval;
}

/*
 * To get the current value of a control.
 * applications initialize the id field of a struct v4l2_control and
 * call this ioctl with a pointer to this structure
 */
static int atomisp_g_ctrl(struct file *file, void *fh,
	struct v4l2_control *control)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_sub_device *asd = atomisp_to_video_pipe(vdev)->asd;
	struct atomisp_device *isp = video_get_drvdata(vdev);
	int i, ret = -EINVAL;

	for (i = 0; i < ctrls_num; i++) {
		if (ci_v4l2_controls[i].id == control->id) {
			ret = 0;
			break;
		}
	}

	if (ret)
		return ret;

	rt_mutex_lock(&isp->mutex);

	switch (control->id) {
	case V4L2_CID_IRIS_ABSOLUTE:
	case V4L2_CID_EXPOSURE_ABSOLUTE:
	case V4L2_CID_FNUMBER_ABSOLUTE:
	case V4L2_CID_2A_STATUS:
	case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
	case V4L2_CID_EXPOSURE:
	case V4L2_CID_EXPOSURE_AUTO:
	case V4L2_CID_SCENE_MODE:
	case V4L2_CID_ISO_SENSITIVITY:
	case V4L2_CID_ISO_SENSITIVITY_AUTO:
	case V4L2_CID_CONTRAST:
	case V4L2_CID_SATURATION:
	case V4L2_CID_SHARPNESS:
	case V4L2_CID_3A_LOCK:
	case V4L2_CID_EXPOSURE_ZONE_NUM:
	case V4L2_CID_TEST_PATTERN:
	case V4L2_CID_TEST_PATTERN_COLOR_R:
	case V4L2_CID_TEST_PATTERN_COLOR_GR:
	case V4L2_CID_TEST_PATTERN_COLOR_GB:
	case V4L2_CID_TEST_PATTERN_COLOR_B:
		rt_mutex_unlock(&isp->mutex);
		return v4l2_g_ctrl(isp->inputs[asd->input_curr].camera->
				   ctrl_handler, control);
	case V4L2_CID_COLORFX:
		ret = atomisp_color_effect(asd, 0, &control->value);
		break;
	case V4L2_CID_ATOMISP_BAD_PIXEL_DETECTION:
		ret = atomisp_bad_pixel(asd, 0, &control->value);
		break;
	case V4L2_CID_ATOMISP_POSTPROCESS_GDC_CAC:
		ret = atomisp_gdc_cac(asd, 0, &control->value);
		break;
	case V4L2_CID_ATOMISP_VIDEO_STABLIZATION:
		ret = atomisp_video_stable(asd, 0, &control->value);
		break;
	case V4L2_CID_ATOMISP_FIXED_PATTERN_NR:
		ret = atomisp_fixed_pattern(asd, 0, &control->value);
		break;
	case V4L2_CID_ATOMISP_FALSE_COLOR_CORRECTION:
		ret = atomisp_false_color(asd, 0, &control->value);
		break;
	case V4L2_CID_ATOMISP_LOW_LIGHT:
		ret = atomisp_low_light(asd, 0, &control->value);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	rt_mutex_unlock(&isp->mutex);
	return ret;
}

/*
 * To change the value of a control.
 * applications initialize the id and value fields of a struct v4l2_control
 * and call this ioctl.
 */
static int atomisp_s_ctrl(struct file *file, void *fh,
			  struct v4l2_control *control)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_sub_device *asd = atomisp_to_video_pipe(vdev)->asd;
	struct atomisp_device *isp = video_get_drvdata(vdev);
	int i, ret = -EINVAL;

	for (i = 0; i < ctrls_num; i++) {
		if (ci_v4l2_controls[i].id == control->id) {
			ret = 0;
			break;
		}
	}

	if (ret)
		return ret;

	rt_mutex_lock(&isp->mutex);
	switch (control->id) {
	case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
	case V4L2_CID_EXPOSURE:
	case V4L2_CID_EXPOSURE_AUTO:
	case V4L2_CID_EXPOSURE_AUTO_PRIORITY:
	case V4L2_CID_SCENE_MODE:
	case V4L2_CID_ISO_SENSITIVITY:
	case V4L2_CID_ISO_SENSITIVITY_AUTO:
	case V4L2_CID_POWER_LINE_FREQUENCY:
	case V4L2_CID_EXPOSURE_METERING:
	case V4L2_CID_CONTRAST:
	case V4L2_CID_SATURATION:
	case V4L2_CID_SHARPNESS:
	case V4L2_CID_3A_LOCK:
	case V4L2_CID_COLORFX_CBCR:
	case V4L2_CID_TEST_PATTERN:
	case V4L2_CID_TEST_PATTERN_COLOR_R:
	case V4L2_CID_TEST_PATTERN_COLOR_GR:
	case V4L2_CID_TEST_PATTERN_COLOR_GB:
	case V4L2_CID_TEST_PATTERN_COLOR_B:
		rt_mutex_unlock(&isp->mutex);
		return v4l2_s_ctrl(NULL,
				   isp->inputs[asd->input_curr].camera->
				   ctrl_handler, control);
	case V4L2_CID_COLORFX:
		ret = atomisp_color_effect(asd, 1, &control->value);
		break;
	case V4L2_CID_ATOMISP_BAD_PIXEL_DETECTION:
		ret = atomisp_bad_pixel(asd, 1, &control->value);
		break;
	case V4L2_CID_ATOMISP_POSTPROCESS_GDC_CAC:
		ret = atomisp_gdc_cac(asd, 1, &control->value);
		break;
	case V4L2_CID_ATOMISP_VIDEO_STABLIZATION:
		ret = atomisp_video_stable(asd, 1, &control->value);
		break;
	case V4L2_CID_ATOMISP_FIXED_PATTERN_NR:
		ret = atomisp_fixed_pattern(asd, 1, &control->value);
		break;
	case V4L2_CID_ATOMISP_FALSE_COLOR_CORRECTION:
		ret = atomisp_false_color(asd, 1, &control->value);
		break;
	case V4L2_CID_REQUEST_FLASH:
		ret = atomisp_flash_enable(asd, control->value);
		break;
	case V4L2_CID_ATOMISP_LOW_LIGHT:
		ret = atomisp_low_light(asd, 1, &control->value);
		break;
	default:
		ret = -EINVAL;
		break;
	}
	rt_mutex_unlock(&isp->mutex);
	return ret;
}
/*
 * To query the attributes of a control.
 * applications set the id field of a struct v4l2_queryctrl and call the
 * this ioctl with a pointer to this structure. The driver fills
 * the rest of the structure.
 */
static int atomisp_queryctl(struct file *file, void *fh,
			    struct v4l2_queryctrl *qc)
{
	int i, ret = -EINVAL;
	struct video_device *vdev = video_devdata(file);
	struct atomisp_sub_device *asd = atomisp_to_video_pipe(vdev)->asd;
	struct atomisp_device *isp = video_get_drvdata(vdev);

	switch (qc->id) {
	case V4L2_CID_FOCUS_ABSOLUTE:
	case V4L2_CID_FOCUS_RELATIVE:
	case V4L2_CID_FOCUS_STATUS:
#ifndef ISP2401
		return v4l2_queryctrl(isp->inputs[asd->input_curr].camera->
				      ctrl_handler, qc);
#else
		if (isp->motor)
			return v4l2_queryctrl(isp->motor->ctrl_handler, qc);
		else
			return v4l2_queryctrl(isp->inputs[asd->input_curr].
					      camera->ctrl_handler, qc);
#endif
	}

	if (qc->id & V4L2_CTRL_FLAG_NEXT_CTRL)
		return ret;

	for (i = 0; i < ctrls_num; i++) {
		if (ci_v4l2_controls[i].id == qc->id) {
			memcpy(qc, &ci_v4l2_controls[i],
			       sizeof(struct v4l2_queryctrl));
			qc->reserved[0] = 0;
			ret = 0;
			break;
		}
	}
	if (ret != 0)
		qc->flags = V4L2_CTRL_FLAG_DISABLED;

	return ret;
}

static int atomisp_camera_g_ext_ctrls(struct file *file, void *fh,
	struct v4l2_ext_controls *c)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_sub_device *asd = atomisp_to_video_pipe(vdev)->asd;
	struct atomisp_device *isp = video_get_drvdata(vdev);
	struct v4l2_control ctrl;
	int i;
	int ret = 0;

	for (i = 0; i < c->count; i++) {
		ctrl.id = c->controls[i].id;
		ctrl.value = c->controls[i].value;
		switch (ctrl.id) {
		case V4L2_CID_EXPOSURE_ABSOLUTE:
		case V4L2_CID_EXPOSURE_AUTO:
		case V4L2_CID_IRIS_ABSOLUTE:
		case V4L2_CID_FNUMBER_ABSOLUTE:
		case V4L2_CID_BIN_FACTOR_HORZ:
		case V4L2_CID_BIN_FACTOR_VERT:
		case V4L2_CID_3A_LOCK:
		case V4L2_CID_TEST_PATTERN:
		case V4L2_CID_TEST_PATTERN_COLOR_R:
		case V4L2_CID_TEST_PATTERN_COLOR_GR:
		case V4L2_CID_TEST_PATTERN_COLOR_GB:
		case V4L2_CID_TEST_PATTERN_COLOR_B:
			/*
			 * Exposure related control will be handled by sensor
			 * driver
			 */
			ret =
			    v4l2_g_ctrl(isp->inputs[asd->input_curr].camera->
					ctrl_handler, &ctrl);
			break;
		case V4L2_CID_FOCUS_ABSOLUTE:
		case V4L2_CID_FOCUS_RELATIVE:
		case V4L2_CID_FOCUS_STATUS:
		case V4L2_CID_FOCUS_AUTO:
#ifndef ISP2401
			if (isp->inputs[asd->input_curr].motor)
#else
			if (isp->motor)
#endif
				ret =
#ifndef ISP2401
				    v4l2_g_ctrl(isp->inputs[asd->input_curr].
						motor->ctrl_handler, &ctrl);
#else
				    v4l2_g_ctrl(isp->motor->ctrl_handler,
						&ctrl);
#endif
			else
				ret =
				    v4l2_g_ctrl(isp->inputs[asd->input_curr].
						camera->ctrl_handler, &ctrl);
			break;
		case V4L2_CID_FLASH_STATUS:
		case V4L2_CID_FLASH_INTENSITY:
		case V4L2_CID_FLASH_TORCH_INTENSITY:
		case V4L2_CID_FLASH_INDICATOR_INTENSITY:
		case V4L2_CID_FLASH_TIMEOUT:
		case V4L2_CID_FLASH_STROBE:
		case V4L2_CID_FLASH_MODE:
		case V4L2_CID_FLASH_STATUS_REGISTER:
			if (isp->flash)
				ret =
				    v4l2_g_ctrl(isp->flash->ctrl_handler,
						&ctrl);
			break;
		case V4L2_CID_ZOOM_ABSOLUTE:
			rt_mutex_lock(&isp->mutex);
			ret = atomisp_digital_zoom(asd, 0, &ctrl.value);
			rt_mutex_unlock(&isp->mutex);
			break;
		case V4L2_CID_G_SKIP_FRAMES:
			ret = v4l2_subdev_call(
				isp->inputs[asd->input_curr].camera,
				sensor, g_skip_frames, (u32 *)&ctrl.value);
			break;
		default:
			ret = -EINVAL;
		}

		if (ret) {
			c->error_idx = i;
			break;
		}
		c->controls[i].value = ctrl.value;
	}
	return ret;
}

/* This ioctl allows the application to get multiple controls by class */
static int atomisp_g_ext_ctrls(struct file *file, void *fh,
	struct v4l2_ext_controls *c)
{
	struct v4l2_control ctrl;
	int i, ret = 0;

	/* input_lock is not need for the Camera releated IOCTLs
	 * The input_lock downgrade the FPS of 3A*/
	ret = atomisp_camera_g_ext_ctrls(file, fh, c);
	if (ret != -EINVAL)
		return ret;

	for (i = 0; i < c->count; i++) {
		ctrl.id = c->controls[i].id;
		ctrl.value = c->controls[i].value;
		ret = atomisp_g_ctrl(file, fh, &ctrl);
		c->controls[i].value = ctrl.value;
		if (ret) {
			c->error_idx = i;
			break;
		}
	}
	return ret;
}

static int atomisp_camera_s_ext_ctrls(struct file *file, void *fh,
	struct v4l2_ext_controls *c)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_sub_device *asd = atomisp_to_video_pipe(vdev)->asd;
	struct atomisp_device *isp = video_get_drvdata(vdev);
	struct v4l2_control ctrl;
	int i;
	int ret = 0;

	for (i = 0; i < c->count; i++) {
		struct v4l2_ctrl *ctr;

		ctrl.id = c->controls[i].id;
		ctrl.value = c->controls[i].value;
		switch (ctrl.id) {
		case V4L2_CID_EXPOSURE_ABSOLUTE:
		case V4L2_CID_EXPOSURE_AUTO:
		case V4L2_CID_EXPOSURE_METERING:
		case V4L2_CID_IRIS_ABSOLUTE:
		case V4L2_CID_FNUMBER_ABSOLUTE:
		case V4L2_CID_VCM_TIMEING:
		case V4L2_CID_VCM_SLEW:
		case V4L2_CID_3A_LOCK:
		case V4L2_CID_TEST_PATTERN:
		case V4L2_CID_TEST_PATTERN_COLOR_R:
		case V4L2_CID_TEST_PATTERN_COLOR_GR:
		case V4L2_CID_TEST_PATTERN_COLOR_GB:
		case V4L2_CID_TEST_PATTERN_COLOR_B:
			ret = v4l2_s_ctrl(NULL,
					  isp->inputs[asd->input_curr].camera->
					  ctrl_handler, &ctrl);
			break;
		case V4L2_CID_FOCUS_ABSOLUTE:
		case V4L2_CID_FOCUS_RELATIVE:
		case V4L2_CID_FOCUS_STATUS:
		case V4L2_CID_FOCUS_AUTO:
#ifndef ISP2401
			if (isp->inputs[asd->input_curr].motor)
#else
			if (isp->motor)
#endif
				ret = v4l2_s_ctrl(NULL,
#ifndef ISP2401
						  isp->inputs[asd->input_curr].
						  motor->ctrl_handler, &ctrl);
#else
						  isp->motor->ctrl_handler,
						  &ctrl);
#endif
			else
				ret = v4l2_s_ctrl(NULL,
						  isp->inputs[asd->input_curr].
						  camera->ctrl_handler, &ctrl);
			break;
		case V4L2_CID_FLASH_STATUS:
		case V4L2_CID_FLASH_INTENSITY:
		case V4L2_CID_FLASH_TORCH_INTENSITY:
		case V4L2_CID_FLASH_INDICATOR_INTENSITY:
		case V4L2_CID_FLASH_TIMEOUT:
		case V4L2_CID_FLASH_STROBE:
		case V4L2_CID_FLASH_MODE:
		case V4L2_CID_FLASH_STATUS_REGISTER:
			rt_mutex_lock(&isp->mutex);
			if (isp->flash) {
				ret =
				    v4l2_s_ctrl(NULL, isp->flash->ctrl_handler,
						&ctrl);
				/* When flash mode is changed we need to reset
				 * flash state */
				if (ctrl.id == V4L2_CID_FLASH_MODE) {
					asd->params.flash_state =
						ATOMISP_FLASH_IDLE;
					asd->params.num_flash_frames = 0;
				}
			}
			rt_mutex_unlock(&isp->mutex);
			break;
		case V4L2_CID_ZOOM_ABSOLUTE:
			rt_mutex_lock(&isp->mutex);
			ret = atomisp_digital_zoom(asd, 1, &ctrl.value);
			rt_mutex_unlock(&isp->mutex);
			break;
		default:
			ctr = v4l2_ctrl_find(&asd->ctrl_handler, ctrl.id);
			if (ctr)
				ret = v4l2_ctrl_s_ctrl(ctr, ctrl.value);
			else
				ret = -EINVAL;
		}

		if (ret) {
			c->error_idx = i;
			break;
		}
		c->controls[i].value = ctrl.value;
	}
	return ret;
}

/* This ioctl allows the application to set multiple controls by class */
static int atomisp_s_ext_ctrls(struct file *file, void *fh,
	struct v4l2_ext_controls *c)
{
	struct v4l2_control ctrl;
	int i, ret = 0;

	/* input_lock is not need for the Camera releated IOCTLs
	 * The input_lock downgrade the FPS of 3A*/
	ret = atomisp_camera_s_ext_ctrls(file, fh, c);
	if (ret != -EINVAL)
		return ret;

	for (i = 0; i < c->count; i++) {
		ctrl.id = c->controls[i].id;
		ctrl.value = c->controls[i].value;
		ret = atomisp_s_ctrl(file, fh, &ctrl);
		c->controls[i].value = ctrl.value;
		if (ret) {
			c->error_idx = i;
			break;
		}
	}
	return ret;
}

/*
 * vidioc_g/s_param are used to switch isp running mode
 */
static int atomisp_g_parm(struct file *file, void *fh,
	struct v4l2_streamparm *parm)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_sub_device *asd = atomisp_to_video_pipe(vdev)->asd;
	struct atomisp_device *isp = video_get_drvdata(vdev);

	if (parm->type != V4L2_BUF_TYPE_VIDEO_CAPTURE) {
		dev_err(isp->dev, "unsupport v4l2 buf type\n");
		return -EINVAL;
	}

	rt_mutex_lock(&isp->mutex);
	parm->parm.capture.capturemode = asd->run_mode->val;
	rt_mutex_unlock(&isp->mutex);

	return 0;
}

static int atomisp_s_parm(struct file *file, void *fh,
	struct v4l2_streamparm *parm)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_device *isp = video_get_drvdata(vdev);
	struct atomisp_sub_device *asd = atomisp_to_video_pipe(vdev)->asd;
	int mode;
	int rval;
	int fps;

	if (parm->type != V4L2_BUF_TYPE_VIDEO_CAPTURE) {
		dev_err(isp->dev, "unsupport v4l2 buf type\n");
		return -EINVAL;
	}

	rt_mutex_lock(&isp->mutex);

	asd->high_speed_mode = false;
	switch (parm->parm.capture.capturemode) {
	case CI_MODE_NONE: {
		struct v4l2_subdev_frame_interval fi = {0};

		fi.interval = parm->parm.capture.timeperframe;

		rval = v4l2_subdev_call(isp->inputs[asd->input_curr].camera,
					video, s_frame_interval, &fi);
		if (!rval)
			parm->parm.capture.timeperframe = fi.interval;

		if (fi.interval.numerator != 0) {
			fps = fi.interval.denominator / fi.interval.numerator;
			if (fps > 30)
				asd->high_speed_mode = true;
		}

		goto out;
	}
	case CI_MODE_VIDEO:
		mode = ATOMISP_RUN_MODE_VIDEO;
		break;
	case CI_MODE_STILL_CAPTURE:
		mode = ATOMISP_RUN_MODE_STILL_CAPTURE;
		break;
	case CI_MODE_CONTINUOUS:
		mode = ATOMISP_RUN_MODE_CONTINUOUS_CAPTURE;
		break;
	case CI_MODE_PREVIEW:
		mode = ATOMISP_RUN_MODE_PREVIEW;
		break;
	default:
		rval = -EINVAL;
		goto out;
	}

	rval = v4l2_ctrl_s_ctrl(asd->run_mode, mode);

out:
	rt_mutex_unlock(&isp->mutex);

	return rval == -ENOIOCTLCMD ? 0 : rval;
}

static int atomisp_s_parm_file(struct file *file, void *fh,
				struct v4l2_streamparm *parm)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_device *isp = video_get_drvdata(vdev);

	if (parm->type != V4L2_BUF_TYPE_VIDEO_OUTPUT) {
		dev_err(isp->dev, "unsupport v4l2 buf type for output\n");
		return -EINVAL;
	}

	rt_mutex_lock(&isp->mutex);
	isp->sw_contex.file_input = true;
	rt_mutex_unlock(&isp->mutex);

	return 0;
}

static long atomisp_vidioc_default(struct file *file, void *fh,
	bool valid_prio, unsigned int cmd, void *arg)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_device *isp = video_get_drvdata(vdev);
	struct atomisp_sub_device *asd;
	bool acc_node;
	int err;

	acc_node = !strncmp(vdev->name, "ATOMISP ISP ACC",
			sizeof(vdev->name));
	if (acc_node)
		asd = atomisp_to_acc_pipe(vdev)->asd;
	else
		asd = atomisp_to_video_pipe(vdev)->asd;

	switch (cmd) {
	case ATOMISP_IOC_G_MOTOR_PRIV_INT_DATA:
	case ATOMISP_IOC_S_EXPOSURE:
	case ATOMISP_IOC_G_SENSOR_CALIBRATION_GROUP:
	case ATOMISP_IOC_G_SENSOR_PRIV_INT_DATA:
	case ATOMISP_IOC_EXT_ISP_CTRL:
	case ATOMISP_IOC_G_SENSOR_AE_BRACKETING_INFO:
	case ATOMISP_IOC_S_SENSOR_AE_BRACKETING_MODE:
	case ATOMISP_IOC_G_SENSOR_AE_BRACKETING_MODE:
	case ATOMISP_IOC_S_SENSOR_AE_BRACKETING_LUT:
	case ATOMISP_IOC_S_SENSOR_EE_CONFIG:
#ifdef ISP2401
	case ATOMISP_IOC_G_UPDATE_EXPOSURE:
#endif
		/* we do not need take isp->mutex for these IOCTLs */
		break;
	default:
		rt_mutex_lock(&isp->mutex);
		break;
	}
	switch (cmd) {
#ifdef ISP2401
	case ATOMISP_IOC_S_SENSOR_RUNMODE:
		err = atomisp_set_sensor_runmode(asd, arg);
		break;

#endif
	case ATOMISP_IOC_G_XNR:
		err = atomisp_xnr(asd, 0, arg);
		break;

	case ATOMISP_IOC_S_XNR:
		err = atomisp_xnr(asd, 1, arg);
		break;

	case ATOMISP_IOC_G_NR:
		err = atomisp_nr(asd, 0, arg);
		break;

	case ATOMISP_IOC_S_NR:
		err = atomisp_nr(asd, 1, arg);
		break;

	case ATOMISP_IOC_G_TNR:
		err = atomisp_tnr(asd, 0, arg);
		break;

	case ATOMISP_IOC_S_TNR:
		err = atomisp_tnr(asd, 1, arg);
		break;

	case ATOMISP_IOC_G_BLACK_LEVEL_COMP:
		err = atomisp_black_level(asd, 0, arg);
		break;

	case ATOMISP_IOC_S_BLACK_LEVEL_COMP:
		err = atomisp_black_level(asd, 1, arg);
		break;

	case ATOMISP_IOC_G_EE:
		err = atomisp_ee(asd, 0, arg);
		break;

	case ATOMISP_IOC_S_EE:
		err = atomisp_ee(asd, 1, arg);
		break;

	case ATOMISP_IOC_G_DIS_STAT:
		err = atomisp_get_dis_stat(asd, arg);
		break;

	case ATOMISP_IOC_G_DVS2_BQ_RESOLUTIONS:
		err = atomisp_get_dvs2_bq_resolutions(asd, arg);
		break;

	case ATOMISP_IOC_S_DIS_COEFS:
		err = atomisp_css_cp_dvs2_coefs(asd, arg,
				&asd->params.css_param, true);
		if (!err && arg)
			asd->params.css_update_params_needed = true;
		break;

	case ATOMISP_IOC_S_DIS_VECTOR:
		err = atomisp_cp_dvs_6axis_config(asd, arg,
				&asd->params.css_param, true);
		if (!err && arg)
			asd->params.css_update_params_needed = true;
		break;

	case ATOMISP_IOC_G_ISP_PARM:
		err = atomisp_param(asd, 0, arg);
		break;

	case ATOMISP_IOC_S_ISP_PARM:
		err = atomisp_param(asd, 1, arg);
		break;

	case ATOMISP_IOC_G_3A_STAT:
		err = atomisp_3a_stat(asd, 0, arg);
		break;

	case ATOMISP_IOC_G_ISP_GAMMA:
		err = atomisp_gamma(asd, 0, arg);
		break;

	case ATOMISP_IOC_S_ISP_GAMMA:
		err = atomisp_gamma(asd, 1, arg);
		break;

	case ATOMISP_IOC_G_ISP_GDC_TAB:
		err = atomisp_gdc_cac_table(asd, 0, arg);
		break;

	case ATOMISP_IOC_S_ISP_GDC_TAB:
		err = atomisp_gdc_cac_table(asd, 1, arg);
		break;

	case ATOMISP_IOC_G_ISP_MACC:
		err = atomisp_macc_table(asd, 0, arg);
		break;

	case ATOMISP_IOC_S_ISP_MACC:
		err = atomisp_macc_table(asd, 1, arg);
		break;

	case ATOMISP_IOC_G_ISP_BAD_PIXEL_DETECTION:
		err = atomisp_bad_pixel_param(asd, 0, arg);
		break;

	case ATOMISP_IOC_S_ISP_BAD_PIXEL_DETECTION:
		err = atomisp_bad_pixel_param(asd, 1, arg);
		break;

	case ATOMISP_IOC_G_ISP_FALSE_COLOR_CORRECTION:
		err = atomisp_false_color_param(asd, 0, arg);
		break;

	case ATOMISP_IOC_S_ISP_FALSE_COLOR_CORRECTION:
		err = atomisp_false_color_param(asd, 1, arg);
		break;

	case ATOMISP_IOC_G_ISP_CTC:
		err = atomisp_ctc(asd, 0, arg);
		break;

	case ATOMISP_IOC_S_ISP_CTC:
		err = atomisp_ctc(asd, 1, arg);
		break;

	case ATOMISP_IOC_G_ISP_WHITE_BALANCE:
		err = atomisp_white_balance_param(asd, 0, arg);
		break;

	case ATOMISP_IOC_S_ISP_WHITE_BALANCE:
		err = atomisp_white_balance_param(asd, 1, arg);
		break;

	case ATOMISP_IOC_G_3A_CONFIG:
		err = atomisp_3a_config_param(asd, 0, arg);
		break;

	case ATOMISP_IOC_S_3A_CONFIG:
		err = atomisp_3a_config_param(asd, 1, arg);
		break;

	case ATOMISP_IOC_S_ISP_FPN_TABLE:
		err = atomisp_fixed_pattern_table(asd, arg);
		break;

	case ATOMISP_IOC_ISP_MAKERNOTE:
		err = atomisp_exif_makernote(asd, arg);
		break;

	case ATOMISP_IOC_G_SENSOR_MODE_DATA:
		err = atomisp_get_sensor_mode_data(asd, arg);
		break;

	case ATOMISP_IOC_G_MOTOR_PRIV_INT_DATA:
#ifndef ISP2401
		if (isp->inputs[asd->input_curr].motor)
#else
		if (isp->motor)
#endif
#ifndef ISP2401
			err = v4l2_subdev_call(
					isp->inputs[asd->input_curr].motor,
					core, ioctl, cmd, arg);
#else
			err = v4l2_subdev_call(
					isp->motor,
					core, ioctl, cmd, arg);
#endif
		else
			err = v4l2_subdev_call(
					isp->inputs[asd->input_curr].camera,
					core, ioctl, cmd, arg);
		break;

	case ATOMISP_IOC_S_EXPOSURE:
	case ATOMISP_IOC_G_SENSOR_CALIBRATION_GROUP:
	case ATOMISP_IOC_G_SENSOR_PRIV_INT_DATA:
	case ATOMISP_IOC_G_SENSOR_AE_BRACKETING_INFO:
	case ATOMISP_IOC_S_SENSOR_AE_BRACKETING_MODE:
	case ATOMISP_IOC_G_SENSOR_AE_BRACKETING_MODE:
	case ATOMISP_IOC_S_SENSOR_AE_BRACKETING_LUT:
#ifdef ISP2401
	case ATOMISP_IOC_G_UPDATE_EXPOSURE:
#endif
		err = v4l2_subdev_call(isp->inputs[asd->input_curr].camera,
					core, ioctl, cmd, arg);
		break;

	case ATOMISP_IOC_ACC_LOAD:
		err = atomisp_acc_load(asd, arg);
		break;

	case ATOMISP_IOC_ACC_LOAD_TO_PIPE:
		err = atomisp_acc_load_to_pipe(asd, arg);
		break;

	case ATOMISP_IOC_ACC_UNLOAD:
		err = atomisp_acc_unload(asd, arg);
		break;

	case ATOMISP_IOC_ACC_START:
		err = atomisp_acc_start(asd, arg);
		break;

	case ATOMISP_IOC_ACC_WAIT:
		err = atomisp_acc_wait(asd, arg);
		break;

	case ATOMISP_IOC_ACC_MAP:
		err = atomisp_acc_map(asd, arg);
		break;

	case ATOMISP_IOC_ACC_UNMAP:
		err = atomisp_acc_unmap(asd, arg);
		break;

	case ATOMISP_IOC_ACC_S_MAPPED_ARG:
		err = atomisp_acc_s_mapped_arg(asd, arg);
		break;

	case ATOMISP_IOC_S_ISP_SHD_TAB:
		err = atomisp_set_shading_table(asd, arg);
		break;

	case ATOMISP_IOC_G_ISP_GAMMA_CORRECTION:
		err = atomisp_gamma_correction(asd, 0, arg);
		break;

	case ATOMISP_IOC_S_ISP_GAMMA_CORRECTION:
		err = atomisp_gamma_correction(asd, 1, arg);
		break;

	case ATOMISP_IOC_S_PARAMETERS:
		err = atomisp_set_parameters(vdev, arg);
		break;

	case ATOMISP_IOC_S_CONT_CAPTURE_CONFIG:
		err = atomisp_offline_capture_configure(asd, arg);
		break;
	case ATOMISP_IOC_G_METADATA:
		err = atomisp_get_metadata(asd, 0, arg);
		break;
	case ATOMISP_IOC_G_METADATA_BY_TYPE:
		err = atomisp_get_metadata_by_type(asd, 0, arg);
		break;
	case ATOMISP_IOC_EXT_ISP_CTRL:
		err = v4l2_subdev_call(isp->inputs[asd->input_curr].camera,
					core, ioctl, cmd, arg);
		break;
	case ATOMISP_IOC_EXP_ID_UNLOCK:
		err = atomisp_exp_id_unlock(asd, arg);
		break;
	case ATOMISP_IOC_EXP_ID_CAPTURE:
		err = atomisp_exp_id_capture(asd, arg);
		break;
	case ATOMISP_IOC_S_ENABLE_DZ_CAPT_PIPE:
		err = atomisp_enable_dz_capt_pipe(asd, arg);
		break;
	case ATOMISP_IOC_G_FORMATS_CONFIG:
		err = atomisp_formats(asd, 0, arg);
		break;

	case ATOMISP_IOC_S_FORMATS_CONFIG:
		err = atomisp_formats(asd, 1, arg);
		break;
	case ATOMISP_IOC_S_EXPOSURE_WINDOW:
		err = atomisp_s_ae_window(asd, arg);
		break;
	case ATOMISP_IOC_S_ACC_STATE:
		err = atomisp_acc_set_state(asd, arg);
		break;
	case ATOMISP_IOC_G_ACC_STATE:
		err = atomisp_acc_get_state(asd, arg);
		break;
	case ATOMISP_IOC_INJECT_A_FAKE_EVENT:
		err = atomisp_inject_a_fake_event(asd, arg);
		break;
	case ATOMISP_IOC_G_INVALID_FRAME_NUM:
		err = atomisp_get_invalid_frame_num(vdev, arg);
		break;
	case ATOMISP_IOC_S_ARRAY_RESOLUTION:
		err = atomisp_set_array_res(asd, arg);
		break;
	default:
		err = -EINVAL;
		break;
	}

	switch (cmd) {
	case ATOMISP_IOC_G_MOTOR_PRIV_INT_DATA:
	case ATOMISP_IOC_S_EXPOSURE:
	case ATOMISP_IOC_G_SENSOR_CALIBRATION_GROUP:
	case ATOMISP_IOC_G_SENSOR_PRIV_INT_DATA:
	case ATOMISP_IOC_EXT_ISP_CTRL:
	case ATOMISP_IOC_G_SENSOR_AE_BRACKETING_INFO:
	case ATOMISP_IOC_S_SENSOR_AE_BRACKETING_MODE:
	case ATOMISP_IOC_G_SENSOR_AE_BRACKETING_MODE:
	case ATOMISP_IOC_S_SENSOR_AE_BRACKETING_LUT:
#ifdef ISP2401
	case ATOMISP_IOC_G_UPDATE_EXPOSURE:
#endif
		break;
	default:
		rt_mutex_unlock(&isp->mutex);
		break;
	}
	return err;
}

const struct v4l2_ioctl_ops atomisp_ioctl_ops = {
	.vidioc_querycap = atomisp_querycap,
	.vidioc_enum_input = atomisp_enum_input,
	.vidioc_g_input = atomisp_g_input,
	.vidioc_s_input = atomisp_s_input,
	.vidioc_queryctrl = atomisp_queryctl,
	.vidioc_s_ctrl = atomisp_s_ctrl,
	.vidioc_g_ctrl = atomisp_g_ctrl,
	.vidioc_s_ext_ctrls = atomisp_s_ext_ctrls,
	.vidioc_g_ext_ctrls = atomisp_g_ext_ctrls,
	.vidioc_enum_fmt_vid_cap = atomisp_enum_fmt_cap,
	.vidioc_try_fmt_vid_cap = atomisp_try_fmt_cap,
	.vidioc_g_fmt_vid_cap = atomisp_g_fmt_cap,
	.vidioc_s_fmt_vid_cap = atomisp_s_fmt_cap,
	.vidioc_reqbufs = atomisp_reqbufs,
	.vidioc_querybuf = atomisp_querybuf,
	.vidioc_qbuf = atomisp_qbuf,
	.vidioc_dqbuf = atomisp_dqbuf,
	.vidioc_streamon = atomisp_streamon,
	.vidioc_streamoff = atomisp_streamoff,
	.vidioc_default = atomisp_vidioc_default,
	.vidioc_s_parm = atomisp_s_parm,
	.vidioc_g_parm = atomisp_g_parm,
};

const struct v4l2_ioctl_ops atomisp_file_ioctl_ops = {
	.vidioc_querycap = atomisp_querycap,
	.vidioc_g_fmt_vid_out = atomisp_g_fmt_file,
	.vidioc_s_fmt_vid_out = atomisp_s_fmt_file,
	.vidioc_s_parm = atomisp_s_parm_file,
	.vidioc_reqbufs = atomisp_reqbufs_file,
	.vidioc_querybuf = atomisp_querybuf_file,
	.vidioc_qbuf = atomisp_qbuf_file,
};
	if (parm->type != V4L2_BUF_TYPE_VIDEO_OUTPUT) {
		dev_err(isp->dev, "unsupport v4l2 buf type for output\n");
		return -EINVAL;
	}

	rt_mutex_lock(&isp->mutex);
	isp->sw_contex.file_input = true;
	rt_mutex_unlock(&isp->mutex);

	return 0;
}

static long atomisp_vidioc_default(struct file *file, void *fh,
	bool valid_prio, unsigned int cmd, void *arg)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_device *isp = video_get_drvdata(vdev);
	struct atomisp_sub_device *asd;
	bool acc_node;
	int err;

	acc_node = !strncmp(vdev->name, "ATOMISP ISP ACC",
			sizeof(vdev->name));
	if (acc_node)
		asd = atomisp_to_acc_pipe(vdev)->asd;
	else
		asd = atomisp_to_video_pipe(vdev)->asd;

	switch (cmd) {
	case ATOMISP_IOC_G_MOTOR_PRIV_INT_DATA:
	case ATOMISP_IOC_S_EXPOSURE:
	case ATOMISP_IOC_G_SENSOR_CALIBRATION_GROUP:
	case ATOMISP_IOC_G_SENSOR_PRIV_INT_DATA:
	case ATOMISP_IOC_EXT_ISP_CTRL:
	case ATOMISP_IOC_G_SENSOR_AE_BRACKETING_INFO:
	case ATOMISP_IOC_S_SENSOR_AE_BRACKETING_MODE:
	case ATOMISP_IOC_G_SENSOR_AE_BRACKETING_MODE:
	case ATOMISP_IOC_S_SENSOR_AE_BRACKETING_LUT:
	case ATOMISP_IOC_S_SENSOR_EE_CONFIG:
#ifdef ISP2401
	case ATOMISP_IOC_G_UPDATE_EXPOSURE:
#endif
		/* we do not need take isp->mutex for these IOCTLs */
		break;
	default:
		rt_mutex_lock(&isp->mutex);
		break;
	}