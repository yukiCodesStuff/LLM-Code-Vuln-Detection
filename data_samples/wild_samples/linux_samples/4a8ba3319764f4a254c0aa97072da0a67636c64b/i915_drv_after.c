{
	struct drm_i915_private *dev_priv = dev->dev_private;
	bool simulated;
	int ret;

	if (!i915.reset)
		return 0;

	intel_reset_gt_powersave(dev);

	mutex_lock(&dev->struct_mutex);

	i915_gem_reset(dev);

	simulated = dev_priv->gpu_error.stop_rings != 0;

	ret = intel_gpu_reset(dev);

	/* Also reset the gpu hangman. */
	if (simulated) {
		DRM_INFO("Simulated gpu hang, resetting stop_rings\n");
		dev_priv->gpu_error.stop_rings = 0;
		if (ret == -ENODEV) {
			DRM_INFO("Reset not implemented, but ignoring "
				 "error for simulated gpu hangs\n");
			ret = 0;
		}
	}
		if (ret) {
			DRM_ERROR("Failed hw init on reset %d\n", ret);
			return ret;
		}

		/*
		 * FIXME: This races pretty badly against concurrent holders of
		 * ring interrupts. This is possible since we've started to drop
		 * dev->struct_mutex in select places when waiting for the gpu.
		 */

		/*
		 * rps/rc6 re-init is necessary to restore state lost after the
		 * reset and the re-install of gt irqs. Skip for ironlake per
		 * previous concerns that it doesn't respond well to some forms
		 * of re-init after reset.
		 */
		if (INTEL_INFO(dev)->gen > 5)
			intel_enable_gt_powersave(dev);
	} else {
		mutex_unlock(&dev->struct_mutex);
	}
static struct drm_driver driver = {
	/* Don't use MTRRs here; the Xserver or userspace app should
	 * deal with them for Intel hardware.
	 */
	.driver_features =
	    DRIVER_USE_AGP |
	    DRIVER_HAVE_IRQ | DRIVER_IRQ_SHARED | DRIVER_GEM | DRIVER_PRIME |
	    DRIVER_RENDER,
	.load = i915_driver_load,
	.unload = i915_driver_unload,
	.open = i915_driver_open,
	.lastclose = i915_driver_lastclose,
	.preclose = i915_driver_preclose,
	.postclose = i915_driver_postclose,
	.set_busid = drm_pci_set_busid,

	/* Used in place of i915_pm_ops for non-DRIVER_MODESET */
	.suspend = i915_suspend_legacy,
	.resume = i915_resume_legacy,

	.device_is_agp = i915_driver_device_is_agp,
#if defined(CONFIG_DEBUG_FS)
	.debugfs_init = i915_debugfs_init,
	.debugfs_cleanup = i915_debugfs_cleanup,
#endif
	.gem_free_object = i915_gem_free_object,
	.gem_vm_ops = &i915_gem_vm_ops,

	.prime_handle_to_fd = drm_gem_prime_handle_to_fd,
	.prime_fd_to_handle = drm_gem_prime_fd_to_handle,
	.gem_prime_export = i915_gem_prime_export,
	.gem_prime_import = i915_gem_prime_import,

	.dumb_create = i915_gem_dumb_create,
	.dumb_map_offset = i915_gem_mmap_gtt,
	.dumb_destroy = drm_gem_dumb_destroy,
	.ioctls = i915_ioctls,
	.fops = &i915_driver_fops,
	.name = DRIVER_NAME,
	.desc = DRIVER_DESC,
	.date = DRIVER_DATE,
	.major = DRIVER_MAJOR,
	.minor = DRIVER_MINOR,
	.patchlevel = DRIVER_PATCHLEVEL,
};

static struct pci_driver i915_pci_driver = {
	.name = DRIVER_NAME,
	.id_table = pciidlist,
	.probe = i915_pci_probe,
	.remove = i915_pci_remove,
	.driver.pm = &i915_pm_ops,
};

static int __init i915_init(void)
{
	driver.num_ioctls = i915_max_ioctl;

	/*
	 * If CONFIG_DRM_I915_KMS is set, default to KMS unless
	 * explicitly disabled with the module pararmeter.
	 *
	 * Otherwise, just follow the parameter (defaulting to off).
	 *
	 * Allow optional vga_text_mode_force boot option to override
	 * the default behavior.
	 */
#if defined(CONFIG_DRM_I915_KMS)
	if (i915.modeset != 0)
		driver.driver_features |= DRIVER_MODESET;
#endif
	if (i915.modeset == 1)
		driver.driver_features |= DRIVER_MODESET;

#ifdef CONFIG_VGA_CONSOLE
	if (vgacon_text_force() && i915.modeset == -1)
		driver.driver_features &= ~DRIVER_MODESET;
#endif

	if (!(driver.driver_features & DRIVER_MODESET)) {
		driver.get_vblank_timestamp = NULL;
#ifndef CONFIG_DRM_I915_UMS
		/* Silently fail loading to not upset userspace. */
		DRM_DEBUG_DRIVER("KMS and UMS disabled.\n");
		return 0;
#endif
	}

	return drm_pci_init(&driver, &i915_pci_driver);
}

static void __exit i915_exit(void)
{
#ifndef CONFIG_DRM_I915_UMS
	if (!(driver.driver_features & DRIVER_MODESET))
		return; /* Never loaded a driver. */
#endif

	drm_pci_exit(&driver, &i915_pci_driver);
}

module_init(i915_init);
module_exit(i915_exit);

MODULE_AUTHOR("Tungsten Graphics, Inc.");
MODULE_AUTHOR("Intel Corporation");

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL and additional rights");