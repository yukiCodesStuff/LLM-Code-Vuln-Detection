{
	struct drm_crtc_state *crtc_state;

	if (plane->state->crtc) {
		crtc_state = state->crtc_states[drm_crtc_index(plane->crtc)];

		if (WARN_ON(!crtc_state))
			return;

		crtc_state->planes_changed = true;
	}