	dev->archdata.dma_ops = ops;
}

static inline int set_arch_dma_coherent_ops(struct device *dev)
{
	dev->archdata.dma_coherent = true;
	set_dma_ops(dev, &coherent_swiotlb_dma_ops);
	return 0;
}
#define set_arch_dma_coherent_ops	set_arch_dma_coherent_ops

/* do not use this function in a driver */
static inline bool is_device_dma_coherent(struct device *dev)
{