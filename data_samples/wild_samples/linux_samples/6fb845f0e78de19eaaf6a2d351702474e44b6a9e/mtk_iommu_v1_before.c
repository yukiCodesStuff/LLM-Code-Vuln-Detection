			"#iommu-cells", 0) {
		int count = of_phandle_iterator_args(&it, iommu_spec.args,
					MAX_PHANDLE_ARGS);
		iommu_spec.np = of_node_get(it.node);
		iommu_spec.args_count = count;

		mtk_iommu_create_mapping(dev, &iommu_spec);
		of_node_put(iommu_spec.np);
	}

	if (!fwspec || fwspec->ops != &mtk_iommu_ops)
		return -ENODEV; /* Not a iommu client device */

	/*
	 * This is a short-term bodge because the ARM DMA code doesn't
	 * understand multi-device groups, but we have to call into it
	 * successfully (and not just rely on a normal IOMMU API attach
	 * here) in order to set the correct DMA API ops on @dev.
	 */
	group = iommu_group_alloc();
	if (IS_ERR(group))
		return PTR_ERR(group);

	err = iommu_group_add_device(group, dev);
	iommu_group_put(group);
	if (err)
		return err;

	data = fwspec->iommu_priv;
	mtk_mapping = data->dev->archdata.iommu;
	err = arm_iommu_attach_device(dev, mtk_mapping);
	if (err) {
		iommu_group_remove_device(dev);
		return err;
	}

	return iommu_device_link(&data->iommu, dev);;
}

static void mtk_iommu_remove_device(struct device *dev)
{
	struct iommu_fwspec *fwspec = dev_iommu_fwspec_get(dev);
	struct mtk_iommu_data *data;

	if (!fwspec || fwspec->ops != &mtk_iommu_ops)
		return;

	data = fwspec->iommu_priv;
	iommu_device_unlink(&data->iommu, dev);

	iommu_group_remove_device(dev);
	iommu_fwspec_free(dev);
}

static int mtk_iommu_hw_init(const struct mtk_iommu_data *data)
{
	u32 regval;
	int ret;

	ret = clk_prepare_enable(data->bclk);
	if (ret) {
		dev_err(data->dev, "Failed to enable iommu bclk(%d)\n", ret);
		return ret;
	}

	regval = F_MMU_CTRL_COHERENT_EN | F_MMU_TF_PROTECT_SEL(2);
	writel_relaxed(regval, data->base + REG_MMU_CTRL_REG);

	regval = F_INT_TRANSLATION_FAULT |
		F_INT_MAIN_MULTI_HIT_FAULT |
		F_INT_INVALID_PA_FAULT |
		F_INT_ENTRY_REPLACEMENT_FAULT |
		F_INT_TABLE_WALK_FAULT |
		F_INT_TLB_MISS_FAULT |
		F_INT_PFH_DMA_FIFO_OVERFLOW |
		F_INT_MISS_DMA_FIFO_OVERFLOW;
	writel_relaxed(regval, data->base + REG_MMU_INT_CONTROL);

	/* protect memory,hw will write here while translation fault */
	writel_relaxed(data->protect_base,
			data->base + REG_MMU_IVRP_PADDR);

	writel_relaxed(F_MMU_DCM_ON, data->base + REG_MMU_DCM);

	if (devm_request_irq(data->dev, data->irq, mtk_iommu_isr, 0,
			     dev_name(data->dev), (void *)data)) {
		writel_relaxed(0, data->base + REG_MMU_PT_BASE_ADDR);
		clk_disable_unprepare(data->bclk);
		dev_err(data->dev, "Failed @ IRQ-%d Request\n", data->irq);
		return -ENODEV;
	}

	return 0;
}

static const struct iommu_ops mtk_iommu_ops = {
	.domain_alloc	= mtk_iommu_domain_alloc,
	.domain_free	= mtk_iommu_domain_free,
	.attach_dev	= mtk_iommu_attach_device,
	.detach_dev	= mtk_iommu_detach_device,
	.map		= mtk_iommu_map,
	.unmap		= mtk_iommu_unmap,
	.iova_to_phys	= mtk_iommu_iova_to_phys,
	.add_device	= mtk_iommu_add_device,
	.remove_device	= mtk_iommu_remove_device,
	.pgsize_bitmap	= ~0UL << MT2701_IOMMU_PAGE_SHIFT,
};

static const struct of_device_id mtk_iommu_of_ids[] = {
	{ .compatible = "mediatek,mt2701-m4u", },
	{}
};

static const struct component_master_ops mtk_iommu_com_ops = {
	.bind		= mtk_iommu_bind,
	.unbind		= mtk_iommu_unbind,
};

static int mtk_iommu_probe(struct platform_device *pdev)
{
	struct mtk_iommu_data		*data;
	struct device			*dev = &pdev->dev;
	struct resource			*res;
	struct component_match		*match = NULL;
	struct of_phandle_args		larb_spec;
	struct of_phandle_iterator	it;
	void				*protect;
	int				larb_nr, ret, err;

	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->dev = dev;

	/* Protect memory. HW will access here while translation fault.*/
	protect = devm_kzalloc(dev, MTK_PROTECT_PA_ALIGN * 2,
			GFP_KERNEL | GFP_DMA);
	if (!protect)
		return -ENOMEM;
	data->protect_base = ALIGN(virt_to_phys(protect), MTK_PROTECT_PA_ALIGN);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	data->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(data->base))
		return PTR_ERR(data->base);

	data->irq = platform_get_irq(pdev, 0);
	if (data->irq < 0)
		return data->irq;

	data->bclk = devm_clk_get(dev, "bclk");
	if (IS_ERR(data->bclk))
		return PTR_ERR(data->bclk);

	larb_nr = 0;
	of_for_each_phandle(&it, err, dev->of_node,
			"mediatek,larbs", NULL, 0) {
		struct platform_device *plarbdev;
		int count = of_phandle_iterator_args(&it, larb_spec.args,
					MAX_PHANDLE_ARGS);

		if (count)
			continue;

		larb_spec.np = of_node_get(it.node);
		if (!of_device_is_available(larb_spec.np))
			continue;

		plarbdev = of_find_device_by_node(larb_spec.np);
		if (!plarbdev) {
			plarbdev = of_platform_device_create(
						larb_spec.np, NULL,
						platform_bus_type.dev_root);
			if (!plarbdev) {
				of_node_put(larb_spec.np);
				return -EPROBE_DEFER;
			}
		}

		data->smi_imu.larb_imu[larb_nr].dev = &plarbdev->dev;
		component_match_add_release(dev, &match, release_of,
					    compare_of, larb_spec.np);
		larb_nr++;
	}

	data->smi_imu.larb_nr = larb_nr;

	platform_set_drvdata(pdev, data);

	ret = mtk_iommu_hw_init(data);
	if (ret)
		return ret;

	ret = iommu_device_sysfs_add(&data->iommu, &pdev->dev, NULL,
				     dev_name(&pdev->dev));
	if (ret)
		return ret;

	iommu_device_set_ops(&data->iommu, &mtk_iommu_ops);

	ret = iommu_device_register(&data->iommu);
	if (ret)
		return ret;

	if (!iommu_present(&platform_bus_type))
		bus_set_iommu(&platform_bus_type,  &mtk_iommu_ops);

	return component_master_add_with_match(dev, &mtk_iommu_com_ops, match);
}