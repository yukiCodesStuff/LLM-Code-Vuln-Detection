	if (crtc_state->base.active || HAS_GMCH_DISPLAY(dev_priv)) {
		/*
		 * We start out with underrun reporting disabled to avoid races.
		 * For correct bookkeeping mark this on active crtcs.
		 *
		 * Also on gmch platforms we dont have any hardware bits to
		 * disable the underrun reporting. Which means we need to start
		 * out with underrun reporting disabled also on inactive pipes,
		 * since otherwise we'll complain about the garbage we read when
		 * e.g. coming up after runtime pm.
		 *
		 * No protection against concurrent access is required - at
		 * worst a fifo underrun happens which also sets this to false.
		 */
		crtc->cpu_fifo_underrun_disabled = true;
		/*
		 * We track the PCH trancoder underrun reporting state
		 * within the crtc. With crtc for pipe A housing the underrun
		 * reporting state for PCH transcoder A, crtc for pipe B housing
		 * it for PCH transcoder B, etc. LPT-H has only PCH transcoder A,
		 * and marking underrun reporting as disabled for the non-existing
		 * PCH transcoders B and C would prevent enabling the south
		 * error interrupt (see cpt_can_enable_serr_int()).
		 */
		if (has_pch_trancoder(dev_priv, crtc->pipe))
			crtc->pch_fifo_underrun_disabled = true;
	}
}

static bool has_bogus_dpll_config(const struct intel_crtc_state *crtc_state)
{
	struct drm_i915_private *dev_priv = to_i915(crtc_state->base.crtc->dev);

	/*
	 * Some SNB BIOSen (eg. ASUS K53SV) are known to misprogram
	 * the hardware when a high res displays plugged in. DPLL P
	 * divider is zero, and the pipe timings are bonkers. We'll
	 * try to disable everything in that case.
	 *
	 * FIXME would be nice to be able to sanitize this state
	 * without several WARNs, but for now let's take the easy
	 * road.
	 */
	return IS_GEN6(dev_priv) &&
		crtc_state->base.active &&
		crtc_state->shared_dpll &&
		crtc_state->port_clock == 0;
}

static void intel_sanitize_encoder(struct intel_encoder *encoder)
{
	struct drm_i915_private *dev_priv = to_i915(encoder->base.dev);
	struct intel_connector *connector;
	struct intel_crtc *crtc = to_intel_crtc(encoder->base.crtc);
	struct intel_crtc_state *crtc_state = crtc ?
		to_intel_crtc_state(crtc->base.state) : NULL;

	/* We need to check both for a crtc link (meaning that the
	 * encoder is active and trying to read from a pipe) and the
	 * pipe itself being active. */
	bool has_active_crtc = crtc_state &&
		crtc_state->base.active;

	if (crtc_state && has_bogus_dpll_config(crtc_state)) {
		DRM_DEBUG_KMS("BIOS has misprogrammed the hardware. Disabling pipe %c\n",
			      pipe_name(crtc->pipe));
		has_active_crtc = false;
	}
	if (connector && !has_active_crtc) {
		DRM_DEBUG_KMS("[ENCODER:%d:%s] has active connectors but no active pipe!\n",
			      encoder->base.base.id,
			      encoder->base.name);

		/* Connector is active, but has no active pipe. This is
		 * fallout from our resume register restoring. Disable
		 * the encoder manually again. */
		if (crtc_state) {
			struct drm_encoder *best_encoder;

			DRM_DEBUG_KMS("[ENCODER:%d:%s] manually disabled\n",
				      encoder->base.base.id,
				      encoder->base.name);

			/* avoid oopsing in case the hooks consult best_encoder */
			best_encoder = connector->base.state->best_encoder;
			connector->base.state->best_encoder = &encoder->base;

			if (encoder->disable)
				encoder->disable(encoder, crtc_state,
						 connector->base.state);
			if (encoder->post_disable)
				encoder->post_disable(encoder, crtc_state,
						      connector->base.state);

			connector->base.state->best_encoder = best_encoder;
		}