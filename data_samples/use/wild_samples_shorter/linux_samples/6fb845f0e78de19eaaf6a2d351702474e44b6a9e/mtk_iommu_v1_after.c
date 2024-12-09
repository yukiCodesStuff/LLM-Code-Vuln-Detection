		iommu_spec.args_count = count;

		mtk_iommu_create_mapping(dev, &iommu_spec);

		/* dev->iommu_fwspec might have changed */
		fwspec = dev_iommu_fwspec_get(dev);

		of_node_put(iommu_spec.np);
	}

	if (!fwspec || fwspec->ops != &mtk_iommu_ops)