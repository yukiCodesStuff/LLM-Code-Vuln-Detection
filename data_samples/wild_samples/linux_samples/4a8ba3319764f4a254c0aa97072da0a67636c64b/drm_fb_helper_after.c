{
	struct drm_fb_helper *fb_helper = info->par;
	struct drm_device *dev = fb_helper->dev;
	struct drm_crtc_helper_funcs *crtc_funcs;
	u16 *red, *green, *blue, *transp;
	struct drm_crtc *crtc;
	int i, j, rc = 0;
	int start;

	if (__drm_modeset_lock_all(dev, !!oops_in_progress)) {
		return -EBUSY;
	}
{
	struct drm_fb_helper *fb_helper = info->par;
	struct drm_device *dev = fb_helper->dev;
	struct drm_mode_set *modeset;
	int ret = 0;
	int i;

	if (__drm_modeset_lock_all(dev, !!oops_in_progress)) {
		return -EBUSY;
	}