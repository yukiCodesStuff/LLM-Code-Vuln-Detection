			     struct v4l2_subdev_format *sink_fmt)
{
	struct csi_priv *priv = v4l2_get_subdevdata(sd);
	struct v4l2_fwnode_endpoint upstream_ep = {};
	const struct imx_media_pixfmt *incc;
	bool is_csi2;
	int ret;

	pinctrl = devm_pinctrl_get_select_default(priv->dev);
	if (IS_ERR(pinctrl)) {
		ret = PTR_ERR(priv->vdev);
		dev_dbg(priv->dev,
			"devm_pinctrl_get_select_default() failed: %d\n", ret);
		if (ret != -ENODEV)
			goto free;
	}

	ret = v4l2_async_register_subdev(&priv->sd);
	if (ret)