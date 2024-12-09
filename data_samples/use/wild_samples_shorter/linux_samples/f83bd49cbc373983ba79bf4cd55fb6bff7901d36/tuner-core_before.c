	if (t->mode != V4L2_TUNER_RADIO)
		return;
	if (fe_tuner_ops->get_status) {
		u32 tuner_status;

		fe_tuner_ops->get_status(&t->fe, &tuner_status);
		if (tuner_status & TUNER_STATUS_LOCKED)
			pr_info("Tuner is locked.\n");
	if (vt->type == t->mode) {
		vt->rxsubchans = V4L2_TUNER_SUB_MONO | V4L2_TUNER_SUB_STEREO;
		if (fe_tuner_ops->get_status) {
			u32 tuner_status;

			fe_tuner_ops->get_status(&t->fe, &tuner_status);
			vt->rxsubchans =
				(tuner_status & TUNER_STATUS_STEREO) ?