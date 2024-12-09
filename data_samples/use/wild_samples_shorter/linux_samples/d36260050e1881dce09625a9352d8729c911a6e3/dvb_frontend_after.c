		}
		for (i = 0; i < tvps->num; i++) {
			err = dtv_property_process_get(
			    fe, &getp, (struct dtv_property *)(tvp + i), file);
			if (err < 0) {
				kfree(tvp);
				return err;
			}