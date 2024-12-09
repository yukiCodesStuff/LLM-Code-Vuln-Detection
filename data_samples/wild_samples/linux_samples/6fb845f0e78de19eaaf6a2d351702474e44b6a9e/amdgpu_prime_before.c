/*
 * Copyright 2012 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * based on nouveau_prime.c
 *
 * Authors: Alex Deucher
 */

/**
 * DOC: PRIME Buffer Sharing
 *
 * The following callback implementations are used for :ref:`sharing GEM buffer
 * objects between different devices via PRIME <prime_buffer_sharing>`.
 */

#include <drm/drmP.h>

#include "amdgpu.h"
#include "amdgpu_display.h"
#include "amdgpu_gem.h"
#include <drm/amdgpu_drm.h>
#include <linux/dma-buf.h>

/**
 * amdgpu_gem_prime_get_sg_table - &drm_driver.gem_prime_get_sg_table
 * implementation
 * @obj: GEM buffer object (BO)
 *
 * Returns:
 * A scatter/gather table for the pinned pages of the BO's memory.
 */
struct sg_table *amdgpu_gem_prime_get_sg_table(struct drm_gem_object *obj)
{
	struct amdgpu_bo *bo = gem_to_amdgpu_bo(obj);
	int npages = bo->tbo.num_pages;

	return drm_prime_pages_to_sg(bo->tbo.ttm->pages, npages);
}
{
	struct reservation_object *resv = attach->dmabuf->resv;
	struct amdgpu_device *adev = dev->dev_private;
	struct amdgpu_bo *bo;
	struct amdgpu_bo_param bp;
	int ret;

	memset(&bp, 0, sizeof(bp));
	bp.size = attach->dmabuf->size;
	bp.byte_align = PAGE_SIZE;
	bp.domain = AMDGPU_GEM_DOMAIN_CPU;
	bp.flags = 0;
	bp.type = ttm_bo_type_sg;
	bp.resv = resv;
	ww_mutex_lock(&resv->lock, NULL);
	ret = amdgpu_bo_create(adev, &bp, &bo);
	if (ret)
		goto error;

	bo->tbo.sg = sg;
	bo->tbo.ttm->sg = sg;
	bo->allowed_domains = AMDGPU_GEM_DOMAIN_GTT;
	bo->preferred_domains = AMDGPU_GEM_DOMAIN_GTT;
	if (attach->dmabuf->ops != &amdgpu_dmabuf_ops)
		bo->prime_shared_count = 1;

	ww_mutex_unlock(&resv->lock);
	return &bo->gem_base;

error:
	ww_mutex_unlock(&resv->lock);
	return ERR_PTR(ret);
}

/**
 * amdgpu_gem_map_attach - &dma_buf_ops.attach implementation
 * @dma_buf: Shared DMA buffer
 * @attach: DMA-buf attachment
 *
 * Makes sure that the shared DMA buffer can be accessed by the target device.
 * For now, simply pins it to the GTT domain, where it should be accessible by
 * all DMA devices.
 *
 * Returns:
 * 0 on success or a negative error code on failure.
 */
static int amdgpu_gem_map_attach(struct dma_buf *dma_buf,
				 struct dma_buf_attachment *attach)
{
	struct drm_gem_object *obj = dma_buf->priv;
	struct amdgpu_bo *bo = gem_to_amdgpu_bo(obj);
	struct amdgpu_device *adev = amdgpu_ttm_adev(bo->tbo.bdev);
	long r;

	r = drm_gem_map_attach(dma_buf, attach);
	if (r)
		return r;

	r = amdgpu_bo_reserve(bo, false);
	if (unlikely(r != 0))
		goto error_detach;


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