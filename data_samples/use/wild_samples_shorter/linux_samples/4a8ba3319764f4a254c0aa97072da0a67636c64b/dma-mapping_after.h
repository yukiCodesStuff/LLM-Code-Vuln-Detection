	dev->archdata.dma_ops = ops;
}

static inline void arch_setup_dma_ops(struct device *dev, u64 dma_base, u64 size,
				      struct iommu_ops *iommu, bool coherent)
{
	dev->archdata.dma_coherent = coherent;
	if (coherent)
		set_dma_ops(dev, &coherent_swiotlb_dma_ops);
}
#define arch_setup_dma_ops	arch_setup_dma_ops

/* do not use this function in a driver */
static inline bool is_device_dma_coherent(struct device *dev)
{