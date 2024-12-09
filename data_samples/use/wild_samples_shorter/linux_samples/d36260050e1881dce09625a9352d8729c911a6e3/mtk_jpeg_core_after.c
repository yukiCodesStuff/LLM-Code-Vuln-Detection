
	jpeg->clk_jdec = devm_clk_get(jpeg->dev, "jpgdec");
	if (IS_ERR(jpeg->clk_jdec))
		return PTR_ERR(jpeg->clk_jdec);

	jpeg->clk_jdec_smi = devm_clk_get(jpeg->dev, "jpgdec-smi");
	if (IS_ERR(jpeg->clk_jdec_smi))
		return PTR_ERR(jpeg->clk_jdec_smi);

	return 0;
}
