{
	unsigned int i;

	isp->sw_contex.file_input = false;
	isp->need_gfx_throttle = true;
	isp->isp_fatal_error = false;
	isp->mipi_frame_size = 0;

	v4l2_ctrl_s_ctrl(asd->run_mode, ATOMISP_RUN_MODE_STILL_CAPTURE);
	memset(&asd->params.css_param, 0, sizeof(asd->params.css_param));
	asd->params.color_effect = V4L2_COLORFX_NONE;
	asd->params.bad_pixel_en = true;
	asd->params.gdc_cac_en = false;
	asd->params.video_dis_en = false;
	asd->params.sc_en = false;
	asd->params.fpn_en = false;
	asd->params.xnr_en = false;
	asd->params.false_color = 0;
	asd->params.online_process = 1;
	asd->params.yuv_ds_en = 0;
	/* s3a grid not enabled for any pipe */