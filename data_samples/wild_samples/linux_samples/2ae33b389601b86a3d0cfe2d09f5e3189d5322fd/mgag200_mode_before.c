	if (edid) {
		drm_mode_connector_update_edid_property(connector, edid);
		ret = drm_add_edid_modes(connector, edid);
		kfree(edid);
	}
	return ret;
}

static int mga_vga_mode_valid(struct drm_connector *connector,
				 struct drm_display_mode *mode)
{
	/* FIXME: Add bandwidth and g200se limitations */

	if (mode->crtc_hdisplay > 2048 || mode->crtc_hsync_start > 4096 ||
	    mode->crtc_hsync_end > 4096 || mode->crtc_htotal > 4096 ||
	    mode->crtc_vdisplay > 2048 || mode->crtc_vsync_start > 4096 ||
	    mode->crtc_vsync_end > 4096 || mode->crtc_vtotal > 4096) {
		return MODE_BAD;
	}

	return MODE_OK;
}
	    mode->crtc_vsync_end > 4096 || mode->crtc_vtotal > 4096) {
		return MODE_BAD;
	}

	return MODE_OK;
}

struct drm_encoder *mga_connector_best_encoder(struct drm_connector
						  *connector)
{
	int enc_id = connector->encoder_ids[0];
	struct drm_mode_object *obj;
	struct drm_encoder *encoder;

	/* pick the encoder ids */
	if (enc_id) {
		obj =
		    drm_mode_object_find(connector->dev, enc_id,
					 DRM_MODE_OBJECT_ENCODER);
		if (!obj)
			return NULL;
		encoder = obj_to_encoder(obj);
		return encoder;
	}
	return NULL;
}

static enum drm_connector_status mga_vga_detect(struct drm_connector
						   *connector, bool force)
{
	return connector_status_connected;
}

static void mga_connector_destroy(struct drm_connector *connector)
{
	struct mga_connector *mga_connector = to_mga_connector(connector);
	mgag200_i2c_destroy(mga_connector->i2c);
	drm_connector_cleanup(connector);
	kfree(connector);
}

struct drm_connector_helper_funcs mga_vga_connector_helper_funcs = {
	.get_modes = mga_vga_get_modes,
	.mode_valid = mga_vga_mode_valid,
	.best_encoder = mga_connector_best_encoder,
};

struct drm_connector_funcs mga_vga_connector_funcs = {
	.dpms = drm_helper_connector_dpms,
	.detect = mga_vga_detect,
	.fill_modes = drm_helper_probe_single_connector_modes,
	.destroy = mga_connector_destroy,
};

static struct drm_connector *mga_vga_init(struct drm_device *dev)
{
	struct drm_connector *connector;
	struct mga_connector *mga_connector;

	mga_connector = kzalloc(sizeof(struct mga_connector), GFP_KERNEL);
	if (!mga_connector)
		return NULL;

	connector = &mga_connector->base;

	drm_connector_init(dev, connector,
			   &mga_vga_connector_funcs, DRM_MODE_CONNECTOR_VGA);

	drm_connector_helper_add(connector, &mga_vga_connector_helper_funcs);

	mga_connector->i2c = mgag200_i2c_create(dev);
	if (!mga_connector->i2c)
		DRM_ERROR("failed to add ddc bus\n");

	return connector;
}