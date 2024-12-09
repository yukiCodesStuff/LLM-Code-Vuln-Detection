	}

	fw = devm_kzalloc(&plat_dev->dev, sizeof(*fw), GFP_KERNEL);
	if (!fw)
		return ERR_PTR(-ENOMEM);
	fw->type = SCP;
	fw->ops = &mtk_vcodec_rproc_msg;
	fw->scp = scp;
