static int mga_vga_mode_valid(struct drm_connector *connector,
				 struct drm_display_mode *mode)
{
	struct drm_device *dev = connector->dev;
	struct mga_device *mdev = (struct mga_device*)dev->dev_private;
	struct mga_fbdev *mfbdev = mdev->mfbdev;
	struct drm_fb_helper *fb_helper = &mfbdev->helper;
	struct drm_fb_helper_connector *fb_helper_conn = NULL;
	int bpp = 32;
	int i = 0;

	/* FIXME: Add bandwidth and g200se limitations */

	if (mode->crtc_hdisplay > 2048 || mode->crtc_hsync_start > 4096 ||
	    mode->crtc_hsync_end > 4096 || mode->crtc_htotal > 4096 ||
		return MODE_BAD;
	}

	/* Validate the mode input by the user */
	for (i = 0; i < fb_helper->connector_count; i++) {
		if (fb_helper->connector_info[i]->connector == connector) {
			/* Found the helper for this connector */
			fb_helper_conn = fb_helper->connector_info[i];
			if (fb_helper_conn->cmdline_mode.specified) {
				if (fb_helper_conn->cmdline_mode.bpp_specified) {
					bpp = fb_helper_conn->cmdline_mode.bpp;
				}
			}
		}
	}

	if ((mode->hdisplay * mode->vdisplay * (bpp/8)) > mdev->mc.vram_size) {
		if (fb_helper_conn)
			fb_helper_conn->cmdline_mode.specified = false;
		return MODE_BAD;
	}

	return MODE_OK;
}

struct drm_encoder *mga_connector_best_encoder(struct drm_connector