	} else {
		/* Clean kernel core startup/idle code to PoC*/
		dcache_clean_range(__mmuoff_data_start, __mmuoff_data_end);
		dcache_clean_range(__idmap_text_start, __idmap_text_end);

		/* Clean kvm setup code to PoC? */
		if (el2_reset_needed())
			dcache_clean_range(__hyp_idmap_text_start, __hyp_idmap_text_end);

		/* make the crash dump kernel image protected again */
		crash_post_resume();

		/*
		 * Tell the hibernation core that we've just restored
		 * the memory
		 */
		in_suspend = 0;

		sleep_cpu = -EINVAL;
		__cpu_suspend_exit();

		/*
		 * Just in case the boot kernel did turn the SSBD
		 * mitigation off behind our back, let's set the state
		 * to what we expect it to be.
		 */
		switch (arm64_get_ssbd_state()) {
		case ARM64_SSBD_FORCE_ENABLE:
		case ARM64_SSBD_KERNEL:
			arm64_set_ssbd_mitigation(true);
		}