
	jpeg->clk_jdec = devm_clk_get(jpeg->dev, "jpgdec");
	if (IS_ERR(jpeg->clk_jdec))
		return -EINVAL;

	jpeg->clk_jdec_smi = devm_clk_get(jpeg->dev, "jpgdec-smi");
	if (IS_ERR(jpeg->clk_jdec_smi))
		return -EINVAL;

	return 0;
}
