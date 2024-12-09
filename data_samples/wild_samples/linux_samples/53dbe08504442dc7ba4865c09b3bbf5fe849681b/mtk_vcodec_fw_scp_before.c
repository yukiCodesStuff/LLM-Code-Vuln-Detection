	if (!scp) {
		dev_err(&plat_dev->dev, "could not get vdec scp handle");
		return ERR_PTR(-EPROBE_DEFER);
	}

	fw = devm_kzalloc(&plat_dev->dev, sizeof(*fw), GFP_KERNEL);
	fw->type = SCP;
	fw->ops = &mtk_vcodec_rproc_msg;
	fw->scp = scp;

	return fw;
}