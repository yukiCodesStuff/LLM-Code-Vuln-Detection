/*
 * Copyright © 2016 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Pierre-Louis Bossart <pierre-louis.bossart@linux.intel.com>
 *    Jerome Anand <jerome.anand@intel.com>
 *    based on VED patches
 *
 */

/**
 * DOC: LPE Audio integration for HDMI or DP playback
 *
 * Motivation:
 * Atom platforms (e.g. valleyview and cherryTrail) integrates a DMA-based
 * interface as an alternative to the traditional HDaudio path. While this
 * mode is unrelated to the LPE aka SST audio engine, the documentation refers
 * to this mode as LPE so we keep this notation for the sake of consistency.
 *
 * The interface is handled by a separate standalone driver maintained in the
 * ALSA subsystem for simplicity. To minimize the interaction between the two
 * subsystems, a bridge is setup between the hdmi-lpe-audio and i915:
 * 1. Create a platform device to share MMIO/IRQ resources
 * 2. Make the platform device child of i915 device for runtime PM.
 * 3. Create IRQ chip to forward the LPE audio irqs.
 * the hdmi-lpe-audio driver probes the lpe audio device and creates a new
 * sound card
 *
 * Threats:
 * Due to the restriction in Linux platform device model, user need manually
 * uninstall the hdmi-lpe-audio driver before uninstalling i915 module,
 * otherwise we might run into use-after-free issues after i915 removes the
 * platform device: even though hdmi-lpe-audio driver is released, the modules
 * is still in "installed" status.
 *
 * Implementation:
 * The MMIO/REG platform resources are created according to the registers
 * specification.
 * When forwarding LPE audio irqs, the flow control handler selection depends
 * on the platform, for example on valleyview handle_simple_irq is enough.
 *
 */

#include <linux/acpi.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/pm_runtime.h>

#include "i915_drv.h"
#include <linux/delay.h>
#include <drm/intel_lpe_audio.h>

#define HAS_LPE_AUDIO(dev_priv) ((dev_priv)->lpe_audio.platdev != NULL)

static struct platform_device *
lpe_audio_platdev_create(struct drm_i915_private *dev_priv)
{
	struct drm_device *dev = &dev_priv->drm;
	struct platform_device_info pinfo = {};
	struct resource *rsc;
	struct platform_device *platdev;
	struct intel_hdmi_lpe_audio_pdata *pdata;

	pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return ERR_PTR(-ENOMEM);

	rsc = kcalloc(2, sizeof(*rsc), GFP_KERNEL);
	if (!rsc) {
		kfree(pdata);
		return ERR_PTR(-ENOMEM);
	}

	rsc[0].start    = rsc[0].end = dev_priv->lpe_audio.irq;
	rsc[0].flags    = IORESOURCE_IRQ;
	rsc[0].name     = "hdmi-lpe-audio-irq";

	rsc[1].start    = pci_resource_start(dev->pdev, 0) +
		I915_HDMI_LPE_AUDIO_BASE;
	rsc[1].end      = pci_resource_start(dev->pdev, 0) +
		I915_HDMI_LPE_AUDIO_BASE + I915_HDMI_LPE_AUDIO_SIZE - 1;
	rsc[1].flags    = IORESOURCE_MEM;
	rsc[1].name     = "hdmi-lpe-audio-mmio";

	pinfo.parent = dev->dev;
	pinfo.name = "hdmi-lpe-audio";
	pinfo.id = -1;
	pinfo.res = rsc;
	pinfo.num_res = 2;
	pinfo.data = pdata;
	pinfo.size_data = sizeof(*pdata);
	pinfo.dma_mask = DMA_BIT_MASK(32);

	pdata->num_pipes = INTEL_INFO(dev_priv)->num_pipes;
	pdata->num_ports = IS_CHERRYVIEW(dev_priv) ? 3 : 2; /* B,C,D or B,C */
	pdata->port[0].pipe = -1;
	pdata->port[1].pipe = -1;
	pdata->port[2].pipe = -1;
	spin_lock_init(&pdata->lpe_audio_slock);

	platdev = platform_device_register_full(&pinfo);
	kfree(rsc);
	kfree(pdata);

	if (IS_ERR(platdev)) {
		DRM_ERROR("Failed to allocate LPE audio platform device\n");
		return platdev;
	}

	pm_runtime_forbid(&platdev->dev);
	pm_runtime_set_active(&platdev->dev);
	pm_runtime_enable(&platdev->dev);

	return platdev;
}