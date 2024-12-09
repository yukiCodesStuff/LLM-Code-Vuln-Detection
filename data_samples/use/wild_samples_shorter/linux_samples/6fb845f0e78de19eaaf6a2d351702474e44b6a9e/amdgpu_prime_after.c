#include "amdgpu_gem.h"
#include <drm/amdgpu_drm.h>
#include <linux/dma-buf.h>
#include <linux/dma-fence-array.h>

/**
 * amdgpu_gem_prime_get_sg_table - &drm_driver.gem_prime_get_sg_table
 * implementation
	return ERR_PTR(ret);
}

static int
__reservation_object_make_exclusive(struct reservation_object *obj)
{
	struct dma_fence **fences;
	unsigned int count;
	int r;

	if (!reservation_object_get_list(obj)) /* no shared fences to convert */
		return 0;

	r = reservation_object_get_fences_rcu(obj, NULL, &count, &fences);
	if (r)
		return r;

	if (count == 0) {
		/* Now that was unexpected. */
	} else if (count == 1) {
		reservation_object_add_excl_fence(obj, fences[0]);
		dma_fence_put(fences[0]);
		kfree(fences);
	} else {
		struct dma_fence_array *array;

		array = dma_fence_array_create(count, fences,
					       dma_fence_context_alloc(1), 0,
					       false);
		if (!array)
			goto err_fences_put;

		reservation_object_add_excl_fence(obj, &array->base);
		dma_fence_put(&array->base);
	}

	return 0;

err_fences_put:
	while (count--)
		dma_fence_put(fences[count]);
	kfree(fences);
	return -ENOMEM;
}

/**
 * amdgpu_gem_map_attach - &dma_buf_ops.attach implementation
 * @dma_buf: Shared DMA buffer
 * @attach: DMA-buf attachment

	if (attach->dev->driver != adev->dev->driver) {
		/*
		 * We only create shared fences for internal use, but importers
		 * of the dmabuf rely on exclusive fences for implicitly
		 * tracking write hazards. As any of the current fences may
		 * correspond to a write, we need to convert all existing
		 * fences on the reservation object into a single exclusive
		 * fence.
		 */
		r = __reservation_object_make_exclusive(bo->tbo.resv);
		if (r)
			goto error_unreserve;
	}

	/* pin buffer into GTT */
	r = amdgpu_bo_pin(bo, AMDGPU_GEM_DOMAIN_GTT);