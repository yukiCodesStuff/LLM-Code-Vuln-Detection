static int mga_vga_mode_valid(struct drm_connector *connector,
				 struct drm_display_mode *mode)
{
	/* FIXME: Add bandwidth and g200se limitations */

	if (mode->crtc_hdisplay > 2048 || mode->crtc_hsync_start > 4096 ||
	    mode->crtc_hsync_end > 4096 || mode->crtc_htotal > 4096 ||
		return MODE_BAD;
	}

	return MODE_OK;
}

struct drm_encoder *mga_connector_best_encoder(struct drm_connector