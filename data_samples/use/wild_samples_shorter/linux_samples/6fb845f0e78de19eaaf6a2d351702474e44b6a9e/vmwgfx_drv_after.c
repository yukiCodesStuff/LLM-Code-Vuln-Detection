 **************************************************************************/
#include <linux/module.h>
#include <linux/console.h>
#include <linux/dma-mapping.h>

#include <drm/drmP.h>
#include "vmwgfx_drv.h"
#include "vmwgfx_binding.h"
#include <drm/ttm/ttm_placement.h>
#include <drm/ttm/ttm_bo_driver.h>
#include <drm/ttm/ttm_module.h>

#define VMWGFX_DRIVER_DESC "Linux drm driver for VMware graphics devices"
#define VMWGFX_CHIP_SVGAII 0
#define VMW_FB_RESERVATION 0
	dev_priv->initial_height = height;
}

/**
 * vmw_assume_iommu - Figure out whether coherent dma-remapping might be
 * taking place.
 * @dev: Pointer to the struct drm_device.
 *
 * Return: true if iommu present, false otherwise.
 */
static bool vmw_assume_iommu(struct drm_device *dev)
{
	const struct dma_map_ops *ops = get_dma_ops(dev->dev);

	return !dma_is_direct(ops) && ops &&
		ops->map_page != dma_direct_map_page;
}

/**
 * vmw_dma_select_mode - Determine how DMA mappings should be set up for this
 * system.
 *
		[vmw_dma_alloc_coherent] = "Using coherent TTM pages.",
		[vmw_dma_map_populate] = "Keeping DMA mappings.",
		[vmw_dma_map_bind] = "Giving up DMA mappings early."};

	if (vmw_force_coherent)
		dev_priv->map_mode = vmw_dma_alloc_coherent;
	else if (vmw_assume_iommu(dev_priv->dev))
		dev_priv->map_mode = vmw_dma_map_populate;
	else if (!vmw_force_iommu)
		dev_priv->map_mode = vmw_dma_phys;
	else if (IS_ENABLED(CONFIG_SWIOTLB) && swiotlb_nr_tbl())
		dev_priv->map_mode = vmw_dma_alloc_coherent;
	else
		dev_priv->map_mode = vmw_dma_map_populate;

	if (dev_priv->map_mode == vmw_dma_map_populate && vmw_restrict_iommu)
		dev_priv->map_mode = vmw_dma_map_bind;

	/* No TTM coherent page pool? FIXME: Ask TTM instead! */
        if (!(IS_ENABLED(CONFIG_SWIOTLB) || IS_ENABLED(CONFIG_INTEL_IOMMU)) &&
	    (dev_priv->map_mode == vmw_dma_alloc_coherent))
		return -EINVAL;

	DRM_INFO("DMA map mode: %s\n", names[dev_priv->map_mode]);
	return 0;
}

/**
 * With 32-bit we can only handle 32 bit PFNs. Optionally set that
 * restriction also for 64-bit systems.
 */
static int vmw_dma_masks(struct vmw_private *dev_priv)
{
	struct drm_device *dev = dev_priv->dev;
	int ret = 0;

	ret = dma_set_mask_and_coherent(dev->dev, DMA_BIT_MASK(64));
	if (dev_priv->map_mode != vmw_dma_phys &&
	    (sizeof(unsigned long) == 4 || vmw_restrict_dma_mask)) {
		DRM_INFO("Restricting DMA addresses to 44 bits.\n");
		return dma_set_mask_and_coherent(dev->dev, DMA_BIT_MASK(44));
	}

	return ret;
}

static int vmw_driver_load(struct drm_device *dev, unsigned long chipset)
{
	struct vmw_private *dev_priv;