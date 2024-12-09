	}
}

static void intel_sanitize_encoder(struct intel_encoder *encoder)
{
	struct drm_i915_private *dev_priv = to_i915(encoder->base.dev);
	struct intel_connector *connector;

	/* We need to check both for a crtc link (meaning that the
	 * encoder is active and trying to read from a pipe) and the
	 * pipe itself being active. */
	bool has_active_crtc = encoder->base.crtc &&
		to_intel_crtc(encoder->base.crtc)->active;

	connector = intel_encoder_find_connector(encoder);
	if (connector && !has_active_crtc) {
		DRM_DEBUG_KMS("[ENCODER:%d:%s] has active connectors but no active pipe!\n",
		/* Connector is active, but has no active pipe. This is
		 * fallout from our resume register restoring. Disable
		 * the encoder manually again. */
		if (encoder->base.crtc) {
			struct drm_crtc_state *crtc_state = encoder->base.crtc->state;

			DRM_DEBUG_KMS("[ENCODER:%d:%s] manually disabled\n",
				      encoder->base.base.id,
				      encoder->base.name);
			if (encoder->disable)
				encoder->disable(encoder, to_intel_crtc_state(crtc_state), connector->base.state);
			if (encoder->post_disable)
				encoder->post_disable(encoder, to_intel_crtc_state(crtc_state), connector->base.state);
		}
		encoder->base.crtc = NULL;

		/* Inconsistent output/port/pipe state happens presumably due to