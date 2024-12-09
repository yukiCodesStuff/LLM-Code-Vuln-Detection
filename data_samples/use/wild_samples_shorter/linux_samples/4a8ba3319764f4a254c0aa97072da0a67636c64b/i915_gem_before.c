i915_gem_create(struct drm_file *file,
		struct drm_device *dev,
		uint64_t size,
		bool dumb,
		uint32_t *handle_p)
{
	struct drm_i915_gem_object *obj;
	int ret;
	if (obj == NULL)
		return -ENOMEM;

	obj->base.dumb = dumb;
	ret = drm_gem_handle_create(file, &obj->base, &handle);
	/* drop reference from allocate - handle holds it now */
	drm_gem_object_unreference_unlocked(&obj->base);
	if (ret)
	args->pitch = ALIGN(args->width * DIV_ROUND_UP(args->bpp, 8), 64);
	args->size = args->pitch * args->height;
	return i915_gem_create(file, dev,
			       args->size, true, &args->handle);
}

/**
 * Creates a new mm object and returns a handle to it.
	struct drm_i915_gem_create *args = data;

	return i915_gem_create(file, dev,
			       args->size, false, &args->handle);
}

static inline int
__copy_to_user_swizzled(char __user *cpu_vaddr,
i915_gem_pwrite_ioctl(struct drm_device *dev, void *data,
		      struct drm_file *file)
{
	struct drm_i915_gem_pwrite *args = data;
	struct drm_i915_gem_object *obj;
	int ret;

			return -EFAULT;
	}

	ret = i915_mutex_lock_interruptible(dev);
	if (ret)
		return ret;

	obj = to_intel_bo(drm_gem_object_lookup(dev, file, args->handle));
	if (&obj->base == NULL) {
		ret = -ENOENT;
	drm_gem_object_unreference(&obj->base);
unlock:
	mutex_unlock(&dev->struct_mutex);
	return ret;
}

int
	drm_gem_free_mmap_offset(&obj->base);
}

static int
i915_gem_mmap_gtt(struct drm_file *file,
		  struct drm_device *dev,
		  uint32_t handle, bool dumb,
		  uint64_t *offset)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct drm_i915_gem_object *obj;
		goto unlock;
	}

	/*
	 * We don't allow dumb mmaps on objects created using another
	 * interface.
	 */
	WARN_ONCE(dumb && !(obj->base.dumb || obj->base.import_attach),
		  "Illegal dumb map of accelerated buffer.\n");

	if (obj->base.size > dev_priv->gtt.mappable_end) {
		ret = -E2BIG;
		goto out;
	}
	return ret;
}

int
i915_gem_dumb_map_offset(struct drm_file *file,
			 struct drm_device *dev,
			 uint32_t handle,
			 uint64_t *offset)
{
	return i915_gem_mmap_gtt(file, dev, handle, true, offset);
}

/**
 * i915_gem_mmap_gtt_ioctl - prepare an object for GTT mmap'ing
 * @dev: DRM device
 * @data: GTT mapping ioctl data
{
	struct drm_i915_gem_mmap_gtt *args = data;

	return i915_gem_mmap_gtt(file, dev, args->handle, false, &args->offset);
}

static inline int
i915_gem_object_is_purgeable(struct drm_i915_gem_object *obj)
	if (!mutex_is_locked(mutex))
		return false;

#if defined(CONFIG_SMP) || defined(CONFIG_DEBUG_MUTEXES)
	return mutex->owner == task;
#else
	/* Since UP may be pre-empted, we cannot assume that we own the lock */
	return false;