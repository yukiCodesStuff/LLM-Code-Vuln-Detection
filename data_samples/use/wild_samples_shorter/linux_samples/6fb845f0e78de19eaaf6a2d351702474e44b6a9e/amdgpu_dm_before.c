	}

	if (connector_type == DRM_MODE_CONNECTOR_HDMIA ||
	    connector_type == DRM_MODE_CONNECTOR_DisplayPort) {
		drm_connector_attach_vrr_capable_property(
			&aconnector->base);
	}
}