{
	struct adv748x_afe *afe = adv748x_sd_to_afe(sd);
	struct adv748x_state *state = adv748x_afe_to_state(afe);
	int ret, signal = V4L2_IN_ST_NO_SIGNAL;

	mutex_lock(&state->mutex);

	if (enable) {