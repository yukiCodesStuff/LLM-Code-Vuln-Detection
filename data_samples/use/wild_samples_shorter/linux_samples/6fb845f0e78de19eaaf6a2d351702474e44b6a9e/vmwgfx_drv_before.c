 **************************************************************************/
#include <linux/module.h>
#include <linux/console.h>

#include <drm/drmP.h>
#include "vmwgfx_drv.h"
#include "vmwgfx_binding.h"
#include <drm/ttm/ttm_placement.h>
#include <drm/ttm/ttm_bo_driver.h>
#include <drm/ttm/ttm_module.h>
#include <linux/intel-iommu.h>

#define VMWGFX_DRIVER_DESC "Linux drm driver for VMware graphics devices"
#define VMWGFX_CHIP_SVGAII 0
#define VMW_FB_RESERVATION 0
	dev_priv->initial_height = height;
}

/**
 * vmw_dma_select_mode - Determine how DMA mappings should be set up for this
 * system.
 *
		[vmw_dma_alloc_coherent] = "Using coherent TTM pages.",
		[vmw_dma_map_populate] = "Keeping DMA mappings.",
		[vmw_dma_map_bind] = "Giving up DMA mappings early."};
#ifdef CONFIG_X86
	const struct dma_map_ops *dma_ops = get_dma_ops(dev_priv->dev->dev);

#ifdef CONFIG_INTEL_IOMMU
	if (intel_iommu_enabled) {
		dev_priv->map_mode = vmw_dma_map_populate;
		goto out_fixup;
	}
#endif

	if (!(vmw_force_iommu || vmw_force_coherent)) {
		dev_priv->map_mode = vmw_dma_phys;
		DRM_INFO("DMA map mode: %s\n", names[dev_priv->map_mode]);
		return 0;
	}

	dev_priv->map_mode = vmw_dma_map_populate;

	if (dma_ops && dma_ops->sync_single_for_cpu)
		dev_priv->map_mode = vmw_dma_alloc_coherent;
#ifdef CONFIG_SWIOTLB
	if (swiotlb_nr_tbl() == 0)
		dev_priv->map_mode = vmw_dma_map_populate;
#endif

#ifdef CONFIG_INTEL_IOMMU
out_fixup:
#endif
	if (dev_priv->map_mode == vmw_dma_map_populate &&
	    vmw_restrict_iommu)
		dev_priv->map_mode = vmw_dma_map_bind;

	if (vmw_force_coherent)
		dev_priv->map_mode = vmw_dma_alloc_coherent;

#if !defined(CONFIG_SWIOTLB) && !defined(CONFIG_INTEL_IOMMU)
	/*
	 * No coherent page pool
	 */
	if (dev_priv->map_mode == vmw_dma_alloc_coherent)
		return -EINVAL;
#endif

#else /* CONFIG_X86 */
	dev_priv->map_mode = vmw_dma_map_populate;
#endif /* CONFIG_X86 */

	DRM_INFO("DMA map mode: %s\n", names[dev_priv->map_mode]);

	return 0;
}

/**
 * With 32-bit we can only handle 32 bit PFNs. Optionally set that
 * restriction also for 64-bit systems.
 */
#ifdef CONFIG_INTEL_IOMMU
static int vmw_dma_masks(struct vmw_private *dev_priv)
{
	struct drm_device *dev = dev_priv->dev;

	if (intel_iommu_enabled &&
	    (sizeof(unsigned long) == 4 || vmw_restrict_dma_mask)) {
		DRM_INFO("Restricting DMA addresses to 44 bits.\n");
		return dma_set_mask(dev->dev, DMA_BIT_MASK(44));
	}
	return 0;
}
#else
static int vmw_dma_masks(struct vmw_private *dev_priv)
{
	return 0;
}
#endif

static int vmw_driver_load(struct drm_device *dev, unsigned long chipset)
{
	struct vmw_private *dev_priv;