#include "amdgpu_gem.h"
#include <drm/amdgpu_drm.h>
#include <linux/dma-buf.h>

/**
 * amdgpu_gem_prime_get_sg_table - &drm_driver.gem_prime_get_sg_table
 * implementation
	return ERR_PTR(ret);
}

/**
 * amdgpu_gem_map_attach - &dma_buf_ops.attach implementation
 * @dma_buf: Shared DMA buffer
 * @attach: DMA-buf attachment

	if (attach->dev->driver != adev->dev->driver) {
		/*
		 * Wait for all shared fences to complete before we switch to future
		 * use of exclusive fence on this prime shared bo.
		 */
		r = reservation_object_wait_timeout_rcu(bo->tbo.resv,
							true, false,
							MAX_SCHEDULE_TIMEOUT);
		if (unlikely(r < 0)) {
			DRM_DEBUG_PRIME("Fence wait failed: %li\n", r);
			goto error_unreserve;
		}
	}

	/* pin buffer into GTT */
	r = amdgpu_bo_pin(bo, AMDGPU_GEM_DOMAIN_GTT);