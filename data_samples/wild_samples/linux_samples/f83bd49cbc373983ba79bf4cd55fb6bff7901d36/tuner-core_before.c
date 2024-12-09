	} else {
		freq = t->tv_freq / 16;
		freq_fraction = (t->tv_freq % 16) * 100 / 16;
	}
	pr_info("Tuner mode:      %s%s\n", p,
		   t->standby ? " on standby mode" : "");
	pr_info("Frequency:       %lu.%02lu MHz\n", freq, freq_fraction);
	pr_info("Standard:        0x%08lx\n", (unsigned long)t->std);
	if (t->mode != V4L2_TUNER_RADIO)
		return;
	if (fe_tuner_ops->get_status) {
		u32 tuner_status;

		fe_tuner_ops->get_status(&t->fe, &tuner_status);
		if (tuner_status & TUNER_STATUS_LOCKED)
			pr_info("Tuner is locked.\n");
		if (tuner_status & TUNER_STATUS_STEREO)
			pr_info("Stereo:          yes\n");
	}
	if (vt->type == t->mode) {
		vt->rxsubchans = V4L2_TUNER_SUB_MONO | V4L2_TUNER_SUB_STEREO;
		if (fe_tuner_ops->get_status) {
			u32 tuner_status;

			fe_tuner_ops->get_status(&t->fe, &tuner_status);
			vt->rxsubchans =
				(tuner_status & TUNER_STATUS_STEREO) ?
				V4L2_TUNER_SUB_STEREO :
				V4L2_TUNER_SUB_MONO;
		}