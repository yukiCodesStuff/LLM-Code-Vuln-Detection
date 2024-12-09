	/* save FP state */
	bl	kvmppc_save_fp

	/*
	 * Set DEC to the smaller of DEC and HDEC, so that we wake
	 * no later than the end of our timeslice (HDEC interrupts
	 * don't wake us from nap).
	bl	kvmhv_accumulate_time
#endif

	/* load up FP state */
	bl	kvmppc_load_fp

	/* Restore guest decrementer */