	if (WARN_ON(!pdev)) {
		of_node_put(node);
		return -EINVAL;
	}
	of_node_put(node);

	jpeg->larb = &pdev->dev;

	jpeg->clk_jdec = devm_clk_get(jpeg->dev, "jpgdec");
	if (IS_ERR(jpeg->clk_jdec))
		return -EINVAL;

	jpeg->clk_jdec_smi = devm_clk_get(jpeg->dev, "jpgdec-smi");
	if (IS_ERR(jpeg->clk_jdec_smi))
		return -EINVAL;

	return 0;
}

static int mtk_jpeg_probe(struct platform_device *pdev)
{
	struct mtk_jpeg_dev *jpeg;
	struct resource *res;
	int dec_irq;
	int ret;

	jpeg = devm_kzalloc(&pdev->dev, sizeof(*jpeg), GFP_KERNEL);
	if (!jpeg)
		return -ENOMEM;

	mutex_init(&jpeg->lock);
	spin_lock_init(&jpeg->hw_lock);
	jpeg->dev = &pdev->dev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	jpeg->dec_reg_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(jpeg->dec_reg_base)) {
		ret = PTR_ERR(jpeg->dec_reg_base);
		return ret;
	}