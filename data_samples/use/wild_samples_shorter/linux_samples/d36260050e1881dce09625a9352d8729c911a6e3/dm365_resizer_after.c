	return 0;
}
static int
resizer_configure_in_continuous_mode(struct vpfe_resizer_device *resizer)
{
	struct device *dev = resizer->crop_resizer.subdev.v4l2_dev->dev;
	struct resizer_params *param = &resizer->config;
	struct vpfe_rsz_config_params *cont_config;
		    ipipeif_source == IPIPEIF_OUTPUT_RESIZER)
			ret = resizer_configure_in_single_shot_mode(resizer);
		else
			ret =  resizer_configure_in_continuous_mode(resizer);
		if (ret)
			return ret;
		ret = config_rsz_hw(resizer, param);
	}