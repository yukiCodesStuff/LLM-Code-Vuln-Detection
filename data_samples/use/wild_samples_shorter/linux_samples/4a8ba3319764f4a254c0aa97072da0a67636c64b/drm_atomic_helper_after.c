	struct drm_crtc_state *crtc_state;

	if (plane->state->crtc) {
		crtc_state = state->crtc_states[drm_crtc_index(plane->state->crtc)];

		if (WARN_ON(!crtc_state))
			return;
