	switch (informat->code) {
	case MEDIA_BUS_FMT_UYVY8_2X8:
		param->rsz_common.src_img_fmt = RSZ_IMG_422;
		param->rsz_common.raw_flip = 0;
		break;

	case MEDIA_BUS_FMT_Y8_1X8:
		param->rsz_common.src_img_fmt = RSZ_IMG_420;
		/* Select y */
		param->rsz_common.y_c = 0;
		param->rsz_common.raw_flip = 0;
		break;

	case MEDIA_BUS_FMT_UV8_1X8:
		param->rsz_common.src_img_fmt = RSZ_IMG_420;
		/* Select y */
		param->rsz_common.y_c = 1;
		param->rsz_common.raw_flip = 0;
		break;

	case MEDIA_BUS_FMT_SGRBG12_1X12:
		param->rsz_common.raw_flip = 1;
		break;

	default:
		param->rsz_common.src_img_fmt = RSZ_IMG_422;
		param->rsz_common.source = IPIPE_DATA;
	}

	param->rsz_common.yuv_y_min = user_config->yuv_y_min;
	param->rsz_common.yuv_y_max = user_config->yuv_y_max;
	param->rsz_common.yuv_c_min = user_config->yuv_c_min;
	param->rsz_common.yuv_c_max = user_config->yuv_c_max;
	param->rsz_common.out_chr_pos = user_config->out_chr_pos;
	param->rsz_common.rsz_seq_crv = user_config->chroma_sample_even;

	return 0;
}
static int
resizer_configure_in_continious_mode(struct vpfe_resizer_device *resizer)
{
	struct device *dev = resizer->crop_resizer.subdev.v4l2_dev->dev;
	struct resizer_params *param = &resizer->config;
	struct vpfe_rsz_config_params *cont_config;
	int line_len_c;
	int line_len;
	int ret;

	if (resizer->resizer_a.output != RESIZER_OUTPUT_MEMORY) {
		dev_err(dev, "enable resizer - Resizer-A\n");
		return -EINVAL;
	}
	    resizer->resizer_b.output == RESIZER_OUTPUT_MEMORY) {
		if (ipipeif_sink == IPIPEIF_INPUT_MEMORY &&
		    ipipeif_source == IPIPEIF_OUTPUT_RESIZER)
			ret = resizer_configure_in_single_shot_mode(resizer);
		else
			ret =  resizer_configure_in_continious_mode(resizer);
		if (ret)
			return ret;
		ret = config_rsz_hw(resizer, param);
	}
	return ret;
}

/*
 * resizer_set_stream() - Enable/Disable streaming on resizer subdev
 * @sd: pointer to v4l2 subdev structure
 * @enable: 1 == Enable, 0 == Disable
 */
static int resizer_set_stream(struct v4l2_subdev *sd, int enable)
{
	struct vpfe_resizer_device *resizer = v4l2_get_subdevdata(sd);

	if (&resizer->crop_resizer.subdev != sd)
		return 0;

	if (resizer->resizer_a.output != RESIZER_OUTPUT_MEMORY)
		return 0;

	switch (enable) {
	case 1:
		if (resizer_do_hw_setup(resizer) < 0)
			return -EINVAL;
		resizer_enable(resizer, enable);
		break;

	case 0:
		resizer_enable(resizer, enable);
		break;
	}

	return 0;
}

/*
 * __resizer_get_format() - helper function for getting resizer format
 * @sd: pointer to subdev.
 * @cfg: V4L2 subdev pad config
 * @pad: pad number.
 * @which: wanted subdev format.
 * Retun wanted mbus frame format.
 */
static struct v4l2_mbus_framefmt *
__resizer_get_format(struct v4l2_subdev *sd, struct v4l2_subdev_pad_config *cfg,
		     unsigned int pad, enum v4l2_subdev_format_whence which)
{
	struct vpfe_resizer_device *resizer = v4l2_get_subdevdata(sd);

	if (which == V4L2_SUBDEV_FORMAT_TRY)
		return v4l2_subdev_get_try_format(sd, cfg, pad);
	if (&resizer->crop_resizer.subdev == sd)
		return &resizer->crop_resizer.formats[pad];
	if (&resizer->resizer_a.subdev == sd)
		return &resizer->resizer_a.formats[pad];
	if (&resizer->resizer_b.subdev == sd)
		return &resizer->resizer_b.formats[pad];
	return NULL;
}

/*
 * resizer_try_format() - Handle try format by pad subdev method
 * @sd: pointer to subdev.
 * @cfg: V4L2 subdev pad config
 * @pad: pad num.
 * @fmt: pointer to v4l2 format structure.
 * @which: wanted subdev format.
 */
static void
resizer_try_format(struct v4l2_subdev *sd, struct v4l2_subdev_pad_config *cfg,
	unsigned int pad, struct v4l2_mbus_framefmt *fmt,
	enum v4l2_subdev_format_whence which)
{
	struct vpfe_resizer_device *resizer = v4l2_get_subdevdata(sd);
	unsigned int max_out_height;
	unsigned int max_out_width;
	unsigned int i;

	if ((&resizer->resizer_a.subdev == sd && pad == RESIZER_PAD_SINK) ||
	    (&resizer->resizer_b.subdev == sd && pad == RESIZER_PAD_SINK) ||
	    (&resizer->crop_resizer.subdev == sd &&
	    (pad == RESIZER_CROP_PAD_SOURCE ||
	    pad == RESIZER_CROP_PAD_SOURCE2 || pad == RESIZER_CROP_PAD_SINK))) {
		for (i = 0; i < ARRAY_SIZE(resizer_input_formats); i++) {
			if (fmt->code == resizer_input_formats[i])
				break;
		}