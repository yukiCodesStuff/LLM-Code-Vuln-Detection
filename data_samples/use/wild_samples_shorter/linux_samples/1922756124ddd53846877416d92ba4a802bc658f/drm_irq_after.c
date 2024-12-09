		    struct drm_file *file_priv)
{
	struct drm_modeset_ctl *modeset = data;
	int ret = 0;
	unsigned int crtc;

	/* If drm_vblank_init() hasn't been called yet, just no-op */
	if (!dev->num_crtcs)
		goto out;