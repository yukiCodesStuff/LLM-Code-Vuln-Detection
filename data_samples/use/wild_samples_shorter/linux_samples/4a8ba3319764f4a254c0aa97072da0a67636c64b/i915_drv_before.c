	if (!i915.reset)
		return 0;

	mutex_lock(&dev->struct_mutex);

	i915_gem_reset(dev);

		 * of re-init after reset.
		 */
		if (INTEL_INFO(dev)->gen > 5)
			intel_reset_gt_powersave(dev);
	} else {
		mutex_unlock(&dev->struct_mutex);
	}

	.gem_prime_import = i915_gem_prime_import,

	.dumb_create = i915_gem_dumb_create,
	.dumb_map_offset = i915_gem_dumb_map_offset,
	.dumb_destroy = drm_gem_dumb_destroy,
	.ioctls = i915_ioctls,
	.fops = &i915_driver_fops,
	.name = DRIVER_NAME,