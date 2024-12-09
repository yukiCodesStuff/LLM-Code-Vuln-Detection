		iommu_spec.args_count = count;

		mtk_iommu_create_mapping(dev, &iommu_spec);
		of_node_put(iommu_spec.np);
	}

	if (!fwspec || fwspec->ops != &mtk_iommu_ops)