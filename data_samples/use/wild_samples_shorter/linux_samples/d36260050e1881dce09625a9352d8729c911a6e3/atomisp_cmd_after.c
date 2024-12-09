}


static void print_csi_rx_errors(enum mipi_port_id port,
				struct atomisp_device *isp)
{
	u32 infos = 0;

}

static struct atomisp_sub_device *
__get_asd_from_port(struct atomisp_device *isp, enum mipi_port_id port)
{
	int i;

	/* Check which isp subdev to send eof */

	spin_lock_irqsave(&isp->lock, flags);
	if (isp->sw_contex.power_state != ATOM_ISP_POWER_UP ||
	    !isp->css_initialized) {
		spin_unlock_irqrestore(&isp->lock, flags);
		return IRQ_HANDLED;
	}
	err = atomisp_css_irq_translate(isp, &irq_infos);
	    (irq_infos & CSS_IRQ_INFO_IF_ERROR)) {
		/* handle mipi receiver error */
		u32 rx_infos;
		enum mipi_port_id port;

		for (port = MIPI_PORT0_ID; port <= MIPI_PORT2_ID;
		     port++) {
			print_csi_rx_errors(port, isp);
			atomisp_css_rx_get_irq_info(port, &rx_infos);
			atomisp_css_rx_clear_irq_info(port, rx_infos);
	}

	if (*value == 0) {
		asd->params.fpn_en = false;
		return 0;
	}

	/* Add function to get black from from sensor with shutter off */
	return 0;
}

enum mipi_port_id __get_mipi_port(struct atomisp_device *isp,
				enum atomisp_camera_port port)
{
	switch (port) {
	case ATOMISP_CAMERA_PORT_PRIMARY:
	return atomisp_update_run_mode(asd);
}

static int configure_pp_input_nop(struct atomisp_sub_device *asd,
				  unsigned int width, unsigned int height)
{
	return 0;
}

static int configure_output_nop(struct atomisp_sub_device *asd,
				unsigned int width, unsigned int height,
				unsigned int min_width,
				enum atomisp_css_frame_format sh_fmt)
{
	return 0;
}

static int get_frame_info_nop(struct atomisp_sub_device *asd,
			      struct atomisp_css_frame_info *finfo)
{
	return 0;
}


	/* if subdev type is SOC camera,we do not need to set DVS */
	if (isp->inputs[asd->input_curr].type == SOC_CAMERA)
		asd->params.video_dis_en = false;

	if (asd->params.video_dis_en &&
	    asd->run_mode->val == ATOMISP_RUN_MODE_VIDEO) {
		/* envelope is 20% of the output resolution */
			ffmt = req_ffmt;
			dev_warn(isp->dev,
			  "can not enable video dis due to sensor limitation.");
			asd->params.video_dis_en = false;
		}
	}
	dev_dbg(isp->dev, "sensor width: %d, height: %d\n",
		ffmt->width, ffmt->height);
	    (ffmt->width < req_ffmt->width || ffmt->height < req_ffmt->height)) {
		dev_warn(isp->dev,
			 "can not enable video dis due to sensor limitation.");
		asd->params.video_dis_en = false;
	}

	atomisp_subdev_set_ffmt(&asd->subdev, fh.pad,
				V4L2_SUBDEV_FORMAT_ACTIVE,

	if (!user_shading_table->enable) {
		atomisp_css_set_shading_table(asd, NULL);
		asd->params.sc_en = false;
		return 0;
	}

	/* If enabling, all tables must be set */
	free_table = asd->params.css_param.shading_table;
	asd->params.css_param.shading_table = shading_table;
	atomisp_css_set_shading_table(asd, shading_table);
	asd->params.sc_en = true;

out:
	if (free_table != NULL)
		atomisp_css_shading_table_free(free_table);
	return 0;
}

static int atomisp_get_pipe_id(struct atomisp_video_pipe *pipe)
{
	struct atomisp_sub_device *asd = pipe->asd;

	if (ATOMISP_USE_YUVPP(asd))