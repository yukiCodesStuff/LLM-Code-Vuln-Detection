{
	struct csi_priv *priv = v4l2_get_subdevdata(sd);
	struct v4l2_fwnode_endpoint upstream_ep = {};
	const struct imx_media_pixfmt *incc;
	bool is_csi2;
	int ret;

	ret = v4l2_subdev_link_validate_default(sd, link,
						source_fmt, sink_fmt);
	if (ret)
		return ret;

	ret = csi_get_upstream_endpoint(priv, &upstream_ep);
	if (ret) {
		v4l2_err(&priv->sd, "failed to find upstream endpoint\n");
		return ret;
	}

	mutex_lock(&priv->lock);

	priv->upstream_ep = upstream_ep;
	is_csi2 = (upstream_ep.bus_type == V4L2_MBUS_CSI2);
	incc = priv->cc[CSI_SINK_PAD];

	if (priv->dest != IPU_CSI_DEST_IDMAC &&
	    (incc->bayer || is_parallel_16bit_bus(&upstream_ep))) {
		v4l2_err(&priv->sd,
			 "bayer/16-bit parallel buses must go to IDMAC pad\n");
		ret = -EINVAL;
		goto out;
	}

	if (is_csi2) {
		int vc_num = 0;
		/*
		 * NOTE! It seems the virtual channels from the mipi csi-2
		 * receiver are used only for routing by the video mux's,
		 * or for hard-wired routing to the CSI's. Once the stream
		 * enters the CSI's however, they are treated internally
		 * in the IPU as virtual channel 0.
		 */
#if 0
		mutex_unlock(&priv->lock);
		vc_num = imx_media_find_mipi_csi2_channel(priv->md,
							  &priv->sd.entity);
		if (vc_num < 0)
			return vc_num;
		mutex_lock(&priv->lock);
#endif
		ipu_csi_set_mipi_datatype(priv->csi, vc_num,
					  &priv->format_mbus[CSI_SINK_PAD]);
	}

	/* select either parallel or MIPI-CSI2 as input to CSI */
	ipu_set_csi_src_mux(priv->ipu, priv->csi_id, is_csi2);
out:
	mutex_unlock(&priv->lock);
	return ret;
}
	if (IS_ERR(pinctrl)) {
		ret = PTR_ERR(priv->vdev);
		dev_dbg(priv->dev,
			"devm_pinctrl_get_select_default() failed: %d\n", ret);
		if (ret != -ENODEV)
			goto free;
	}