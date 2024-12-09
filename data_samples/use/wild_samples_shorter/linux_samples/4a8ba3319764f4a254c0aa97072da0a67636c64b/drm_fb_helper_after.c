	int i, j, rc = 0;
	int start;

	if (__drm_modeset_lock_all(dev, !!oops_in_progress)) {
		return -EBUSY;
	}
	if (!drm_fb_helper_is_bound(fb_helper)) {
		drm_modeset_unlock_all(dev);
		return -EBUSY;
	}
	int ret = 0;
	int i;

	if (__drm_modeset_lock_all(dev, !!oops_in_progress)) {
		return -EBUSY;
	}
	if (!drm_fb_helper_is_bound(fb_helper)) {
		drm_modeset_unlock_all(dev);
		return -EBUSY;
	}