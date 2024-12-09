// SPDX-License-Identifier: GPL-2.0 OR MIT
/**************************************************************************
 *
 * Copyright 2009-2016 VMware, Inc., Palo Alto, CA., USA
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/
#include <linux/module.h>
#include <linux/console.h>
#include <linux/dma-mapping.h>

#include <drm/drmP.h>
#include "vmwgfx_drv.h"
#include "vmwgfx_binding.h"
#include "ttm_object.h"
#include <drm/ttm/ttm_placement.h>
#include <drm/ttm/ttm_bo_driver.h>
#include <drm/ttm/ttm_module.h>

#define VMWGFX_DRIVER_DESC "Linux drm driver for VMware graphics devices"
#define VMWGFX_CHIP_SVGAII 0
#define VMW_FB_RESERVATION 0

#define VMW_MIN_INITIAL_WIDTH 800
#define VMW_MIN_INITIAL_HEIGHT 600

#ifndef VMWGFX_GIT_VERSION
#define VMWGFX_GIT_VERSION "Unknown"
#endif

#define VMWGFX_REPO "In Tree"

#define VMWGFX_VALIDATION_MEM_GRAN (16*PAGE_SIZE)


/**
 * Fully encoded drm commands. Might move to vmw_drm.h
 */

#define DRM_IOCTL_VMW_GET_PARAM					\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_GET_PARAM,		\
		 struct drm_vmw_getparam_arg)
#define DRM_IOCTL_VMW_ALLOC_DMABUF				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_ALLOC_DMABUF,	\
		union drm_vmw_alloc_dmabuf_arg)
#define DRM_IOCTL_VMW_UNREF_DMABUF				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_UNREF_DMABUF,	\
		struct drm_vmw_unref_dmabuf_arg)
#define DRM_IOCTL_VMW_CURSOR_BYPASS				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_CURSOR_BYPASS,	\
		 struct drm_vmw_cursor_bypass_arg)

#define DRM_IOCTL_VMW_CONTROL_STREAM				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_CONTROL_STREAM,	\
		 struct drm_vmw_control_stream_arg)
#define DRM_IOCTL_VMW_CLAIM_STREAM				\
	DRM_IOR(DRM_COMMAND_BASE + DRM_VMW_CLAIM_STREAM,	\
		 struct drm_vmw_stream_arg)
#define DRM_IOCTL_VMW_UNREF_STREAM				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_UNREF_STREAM,	\
		 struct drm_vmw_stream_arg)

#define DRM_IOCTL_VMW_CREATE_CONTEXT				\
	DRM_IOR(DRM_COMMAND_BASE + DRM_VMW_CREATE_CONTEXT,	\
		struct drm_vmw_context_arg)
#define DRM_IOCTL_VMW_UNREF_CONTEXT				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_UNREF_CONTEXT,	\
		struct drm_vmw_context_arg)
#define DRM_IOCTL_VMW_CREATE_SURFACE				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_CREATE_SURFACE,	\
		 union drm_vmw_surface_create_arg)
#define DRM_IOCTL_VMW_UNREF_SURFACE				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_UNREF_SURFACE,	\
		 struct drm_vmw_surface_arg)
#define DRM_IOCTL_VMW_REF_SURFACE				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_REF_SURFACE,	\
		 union drm_vmw_surface_reference_arg)
#define DRM_IOCTL_VMW_EXECBUF					\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_EXECBUF,		\
		struct drm_vmw_execbuf_arg)
#define DRM_IOCTL_VMW_GET_3D_CAP				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_GET_3D_CAP,		\
		 struct drm_vmw_get_3d_cap_arg)
#define DRM_IOCTL_VMW_FENCE_WAIT				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_FENCE_WAIT,		\
		 struct drm_vmw_fence_wait_arg)
#define DRM_IOCTL_VMW_FENCE_SIGNALED				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_FENCE_SIGNALED,	\
		 struct drm_vmw_fence_signaled_arg)
#define DRM_IOCTL_VMW_FENCE_UNREF				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_FENCE_UNREF,		\
		 struct drm_vmw_fence_arg)
#define DRM_IOCTL_VMW_FENCE_EVENT				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_FENCE_EVENT,		\
		 struct drm_vmw_fence_event_arg)
#define DRM_IOCTL_VMW_PRESENT					\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_PRESENT,		\
		 struct drm_vmw_present_arg)
#define DRM_IOCTL_VMW_PRESENT_READBACK				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_PRESENT_READBACK,	\
		 struct drm_vmw_present_readback_arg)
#define DRM_IOCTL_VMW_UPDATE_LAYOUT				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_UPDATE_LAYOUT,	\
		 struct drm_vmw_update_layout_arg)
#define DRM_IOCTL_VMW_CREATE_SHADER				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_CREATE_SHADER,	\
		 struct drm_vmw_shader_create_arg)
#define DRM_IOCTL_VMW_UNREF_SHADER				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_UNREF_SHADER,	\
		 struct drm_vmw_shader_arg)
#define DRM_IOCTL_VMW_GB_SURFACE_CREATE				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_GB_SURFACE_CREATE,	\
		 union drm_vmw_gb_surface_create_arg)
#define DRM_IOCTL_VMW_GB_SURFACE_REF				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_GB_SURFACE_REF,	\
		 union drm_vmw_gb_surface_reference_arg)
#define DRM_IOCTL_VMW_SYNCCPU					\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_SYNCCPU,		\
		 struct drm_vmw_synccpu_arg)
#define DRM_IOCTL_VMW_CREATE_EXTENDED_CONTEXT			\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_CREATE_EXTENDED_CONTEXT,	\
		struct drm_vmw_context_arg)
#define DRM_IOCTL_VMW_GB_SURFACE_CREATE_EXT				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_GB_SURFACE_CREATE_EXT,	\
		union drm_vmw_gb_surface_create_ext_arg)
#define DRM_IOCTL_VMW_GB_SURFACE_REF_EXT				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_GB_SURFACE_REF_EXT,		\
		union drm_vmw_gb_surface_reference_ext_arg)

/**
 * The core DRM version of this macro doesn't account for
 * DRM_COMMAND_BASE.
 */

#define VMW_IOCTL_DEF(ioctl, func, flags) \
  [DRM_IOCTL_NR(DRM_IOCTL_##ioctl) - DRM_COMMAND_BASE] = {DRM_IOCTL_##ioctl, flags, func}

/**
 * Ioctl definitions.
 */

static const struct drm_ioctl_desc vmw_ioctls[] = {
	VMW_IOCTL_DEF(VMW_GET_PARAM, vmw_getparam_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_ALLOC_DMABUF, vmw_bo_alloc_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_UNREF_DMABUF, vmw_bo_unref_ioctl,
		      DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_CURSOR_BYPASS,
		      vmw_kms_cursor_bypass_ioctl,
		      DRM_MASTER),

	VMW_IOCTL_DEF(VMW_CONTROL_STREAM, vmw_overlay_ioctl,
		      DRM_MASTER),
	VMW_IOCTL_DEF(VMW_CLAIM_STREAM, vmw_stream_claim_ioctl,
		      DRM_MASTER),
	VMW_IOCTL_DEF(VMW_UNREF_STREAM, vmw_stream_unref_ioctl,
		      DRM_MASTER),

	VMW_IOCTL_DEF(VMW_CREATE_CONTEXT, vmw_context_define_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_UNREF_CONTEXT, vmw_context_destroy_ioctl,
		      DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_CREATE_SURFACE, vmw_surface_define_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_UNREF_SURFACE, vmw_surface_destroy_ioctl,
		      DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_REF_SURFACE, vmw_surface_reference_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_EXECBUF, NULL, DRM_AUTH |
		      DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_FENCE_WAIT, vmw_fence_obj_wait_ioctl,
		      DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_FENCE_SIGNALED,
		      vmw_fence_obj_signaled_ioctl,
		      DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_FENCE_UNREF, vmw_fence_obj_unref_ioctl,
		      DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_FENCE_EVENT, vmw_fence_event_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_GET_3D_CAP, vmw_get_cap_3d_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),

	/* these allow direct access to the framebuffers mark as master only */
	VMW_IOCTL_DEF(VMW_PRESENT, vmw_present_ioctl,
		      DRM_MASTER | DRM_AUTH),
	VMW_IOCTL_DEF(VMW_PRESENT_READBACK,
		      vmw_present_readback_ioctl,
		      DRM_MASTER | DRM_AUTH),
	/*
	 * The permissions of the below ioctl are overridden in
	 * vmw_generic_ioctl(). We require either
	 * DRM_MASTER or capable(CAP_SYS_ADMIN).
	 */
	VMW_IOCTL_DEF(VMW_UPDATE_LAYOUT,
		      vmw_kms_update_layout_ioctl,
		      DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_CREATE_SHADER,
		      vmw_shader_define_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_UNREF_SHADER,
		      vmw_shader_destroy_ioctl,
		      DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_GB_SURFACE_CREATE,
		      vmw_gb_surface_define_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_GB_SURFACE_REF,
		      vmw_gb_surface_reference_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_SYNCCPU,
		      vmw_user_bo_synccpu_ioctl,
		      DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_CREATE_EXTENDED_CONTEXT,
		      vmw_extended_context_define_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_GB_SURFACE_CREATE_EXT,
		      vmw_gb_surface_define_ext_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_GB_SURFACE_REF_EXT,
		      vmw_gb_surface_reference_ext_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
};

static const struct pci_device_id vmw_pci_id_list[] = {
	{0x15ad, 0x0405, PCI_ANY_ID, PCI_ANY_ID, 0, 0, VMWGFX_CHIP_SVGAII},
	{0, 0, 0}
// SPDX-License-Identifier: GPL-2.0 OR MIT
/**************************************************************************
 *
 * Copyright 2009-2016 VMware, Inc., Palo Alto, CA., USA
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/
#include <linux/module.h>
#include <linux/console.h>
#include <linux/dma-mapping.h>

#include <drm/drmP.h>
#include "vmwgfx_drv.h"
#include "vmwgfx_binding.h"
#include "ttm_object.h"
#include <drm/ttm/ttm_placement.h>
#include <drm/ttm/ttm_bo_driver.h>
#include <drm/ttm/ttm_module.h>

#define VMWGFX_DRIVER_DESC "Linux drm driver for VMware graphics devices"
#define VMWGFX_CHIP_SVGAII 0
#define VMW_FB_RESERVATION 0

#define VMW_MIN_INITIAL_WIDTH 800
#define VMW_MIN_INITIAL_HEIGHT 600

#ifndef VMWGFX_GIT_VERSION
#define VMWGFX_GIT_VERSION "Unknown"
#endif

#define VMWGFX_REPO "In Tree"

#define VMWGFX_VALIDATION_MEM_GRAN (16*PAGE_SIZE)


/**
 * Fully encoded drm commands. Might move to vmw_drm.h
 */

#define DRM_IOCTL_VMW_GET_PARAM					\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_GET_PARAM,		\
		 struct drm_vmw_getparam_arg)
#define DRM_IOCTL_VMW_ALLOC_DMABUF				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_ALLOC_DMABUF,	\
		union drm_vmw_alloc_dmabuf_arg)
#define DRM_IOCTL_VMW_UNREF_DMABUF				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_UNREF_DMABUF,	\
		struct drm_vmw_unref_dmabuf_arg)
#define DRM_IOCTL_VMW_CURSOR_BYPASS				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_CURSOR_BYPASS,	\
		 struct drm_vmw_cursor_bypass_arg)

#define DRM_IOCTL_VMW_CONTROL_STREAM				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_CONTROL_STREAM,	\
		 struct drm_vmw_control_stream_arg)
#define DRM_IOCTL_VMW_CLAIM_STREAM				\
	DRM_IOR(DRM_COMMAND_BASE + DRM_VMW_CLAIM_STREAM,	\
		 struct drm_vmw_stream_arg)
#define DRM_IOCTL_VMW_UNREF_STREAM				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_UNREF_STREAM,	\
		 struct drm_vmw_stream_arg)

#define DRM_IOCTL_VMW_CREATE_CONTEXT				\
	DRM_IOR(DRM_COMMAND_BASE + DRM_VMW_CREATE_CONTEXT,	\
		struct drm_vmw_context_arg)
#define DRM_IOCTL_VMW_UNREF_CONTEXT				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_UNREF_CONTEXT,	\
		struct drm_vmw_context_arg)
#define DRM_IOCTL_VMW_CREATE_SURFACE				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_CREATE_SURFACE,	\
		 union drm_vmw_surface_create_arg)
#define DRM_IOCTL_VMW_UNREF_SURFACE				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_UNREF_SURFACE,	\
		 struct drm_vmw_surface_arg)
#define DRM_IOCTL_VMW_REF_SURFACE				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_REF_SURFACE,	\
		 union drm_vmw_surface_reference_arg)
#define DRM_IOCTL_VMW_EXECBUF					\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_EXECBUF,		\
		struct drm_vmw_execbuf_arg)
#define DRM_IOCTL_VMW_GET_3D_CAP				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_GET_3D_CAP,		\
		 struct drm_vmw_get_3d_cap_arg)
#define DRM_IOCTL_VMW_FENCE_WAIT				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_FENCE_WAIT,		\
		 struct drm_vmw_fence_wait_arg)
#define DRM_IOCTL_VMW_FENCE_SIGNALED				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_FENCE_SIGNALED,	\
		 struct drm_vmw_fence_signaled_arg)
#define DRM_IOCTL_VMW_FENCE_UNREF				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_FENCE_UNREF,		\
		 struct drm_vmw_fence_arg)
#define DRM_IOCTL_VMW_FENCE_EVENT				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_FENCE_EVENT,		\
		 struct drm_vmw_fence_event_arg)
#define DRM_IOCTL_VMW_PRESENT					\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_PRESENT,		\
		 struct drm_vmw_present_arg)
#define DRM_IOCTL_VMW_PRESENT_READBACK				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_PRESENT_READBACK,	\
		 struct drm_vmw_present_readback_arg)
#define DRM_IOCTL_VMW_UPDATE_LAYOUT				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_UPDATE_LAYOUT,	\
		 struct drm_vmw_update_layout_arg)
#define DRM_IOCTL_VMW_CREATE_SHADER				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_CREATE_SHADER,	\
		 struct drm_vmw_shader_create_arg)
#define DRM_IOCTL_VMW_UNREF_SHADER				\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_UNREF_SHADER,	\
		 struct drm_vmw_shader_arg)
#define DRM_IOCTL_VMW_GB_SURFACE_CREATE				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_GB_SURFACE_CREATE,	\
		 union drm_vmw_gb_surface_create_arg)
#define DRM_IOCTL_VMW_GB_SURFACE_REF				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_GB_SURFACE_REF,	\
		 union drm_vmw_gb_surface_reference_arg)
#define DRM_IOCTL_VMW_SYNCCPU					\
	DRM_IOW(DRM_COMMAND_BASE + DRM_VMW_SYNCCPU,		\
		 struct drm_vmw_synccpu_arg)
#define DRM_IOCTL_VMW_CREATE_EXTENDED_CONTEXT			\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_CREATE_EXTENDED_CONTEXT,	\
		struct drm_vmw_context_arg)
#define DRM_IOCTL_VMW_GB_SURFACE_CREATE_EXT				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_GB_SURFACE_CREATE_EXT,	\
		union drm_vmw_gb_surface_create_ext_arg)
#define DRM_IOCTL_VMW_GB_SURFACE_REF_EXT				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_VMW_GB_SURFACE_REF_EXT,		\
		union drm_vmw_gb_surface_reference_ext_arg)

/**
 * The core DRM version of this macro doesn't account for
 * DRM_COMMAND_BASE.
 */

#define VMW_IOCTL_DEF(ioctl, func, flags) \
  [DRM_IOCTL_NR(DRM_IOCTL_##ioctl) - DRM_COMMAND_BASE] = {DRM_IOCTL_##ioctl, flags, func}

/**
 * Ioctl definitions.
 */

static const struct drm_ioctl_desc vmw_ioctls[] = {
	VMW_IOCTL_DEF(VMW_GET_PARAM, vmw_getparam_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_ALLOC_DMABUF, vmw_bo_alloc_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_UNREF_DMABUF, vmw_bo_unref_ioctl,
		      DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_CURSOR_BYPASS,
		      vmw_kms_cursor_bypass_ioctl,
		      DRM_MASTER),

	VMW_IOCTL_DEF(VMW_CONTROL_STREAM, vmw_overlay_ioctl,
		      DRM_MASTER),
	VMW_IOCTL_DEF(VMW_CLAIM_STREAM, vmw_stream_claim_ioctl,
		      DRM_MASTER),
	VMW_IOCTL_DEF(VMW_UNREF_STREAM, vmw_stream_unref_ioctl,
		      DRM_MASTER),

	VMW_IOCTL_DEF(VMW_CREATE_CONTEXT, vmw_context_define_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_UNREF_CONTEXT, vmw_context_destroy_ioctl,
		      DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_CREATE_SURFACE, vmw_surface_define_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_UNREF_SURFACE, vmw_surface_destroy_ioctl,
		      DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_REF_SURFACE, vmw_surface_reference_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_EXECBUF, NULL, DRM_AUTH |
		      DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_FENCE_WAIT, vmw_fence_obj_wait_ioctl,
		      DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_FENCE_SIGNALED,
		      vmw_fence_obj_signaled_ioctl,
		      DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_FENCE_UNREF, vmw_fence_obj_unref_ioctl,
		      DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_FENCE_EVENT, vmw_fence_event_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_GET_3D_CAP, vmw_get_cap_3d_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),

	/* these allow direct access to the framebuffers mark as master only */
	VMW_IOCTL_DEF(VMW_PRESENT, vmw_present_ioctl,
		      DRM_MASTER | DRM_AUTH),
	VMW_IOCTL_DEF(VMW_PRESENT_READBACK,
		      vmw_present_readback_ioctl,
		      DRM_MASTER | DRM_AUTH),
	/*
	 * The permissions of the below ioctl are overridden in
	 * vmw_generic_ioctl(). We require either
	 * DRM_MASTER or capable(CAP_SYS_ADMIN).
	 */
	VMW_IOCTL_DEF(VMW_UPDATE_LAYOUT,
		      vmw_kms_update_layout_ioctl,
		      DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_CREATE_SHADER,
		      vmw_shader_define_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_UNREF_SHADER,
		      vmw_shader_destroy_ioctl,
		      DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_GB_SURFACE_CREATE,
		      vmw_gb_surface_define_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_GB_SURFACE_REF,
		      vmw_gb_surface_reference_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_SYNCCPU,
		      vmw_user_bo_synccpu_ioctl,
		      DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_CREATE_EXTENDED_CONTEXT,
		      vmw_extended_context_define_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_GB_SURFACE_CREATE_EXT,
		      vmw_gb_surface_define_ext_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
	VMW_IOCTL_DEF(VMW_GB_SURFACE_REF_EXT,
		      vmw_gb_surface_reference_ext_ioctl,
		      DRM_AUTH | DRM_RENDER_ALLOW),
};

static const struct pci_device_id vmw_pci_id_list[] = {
	{0x15ad, 0x0405, PCI_ANY_ID, PCI_ANY_ID, 0, 0, VMWGFX_CHIP_SVGAII},
	{0, 0, 0}
	    height > dev_priv->fb_max_height) {

		/*
		 * This is a host error and shouldn't occur.
		 */

		width = VMW_MIN_INITIAL_WIDTH;
		height = VMW_MIN_INITIAL_HEIGHT;
	}

	dev_priv->initial_width = width;
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
 * @dev_priv: Pointer to a struct vmw_private
 *
 * This functions tries to determine the IOMMU setup and what actions
 * need to be taken by the driver to make system pages visible to the
 * device.
 * If this function decides that DMA is not possible, it returns -EINVAL.
 * The driver may then try to disable features of the device that require
 * DMA.
 */
static int vmw_dma_select_mode(struct vmw_private *dev_priv)
{
	static const char *names[vmw_dma_map_max] = {
		[vmw_dma_phys] = "Using physical TTM page addresses.",
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
	static const char *names[vmw_dma_map_max] = {
		[vmw_dma_phys] = "Using physical TTM page addresses.",
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
 * vmw_dma_masks - set required page- and dma masks
 *
 * @dev: Pointer to struct drm-device
 *
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
	static const char *names[vmw_dma_map_max] = {
		[vmw_dma_phys] = "Using physical TTM page addresses.",
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
 * vmw_dma_masks - set required page- and dma masks
 *
 * @dev: Pointer to struct drm-device
 *
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