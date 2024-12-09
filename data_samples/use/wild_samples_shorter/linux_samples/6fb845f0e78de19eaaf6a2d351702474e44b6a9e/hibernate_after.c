		dcache_clean_range(__idmap_text_start, __idmap_text_end);

		/* Clean kvm setup code to PoC? */
		if (el2_reset_needed()) {
			dcache_clean_range(__hyp_idmap_text_start, __hyp_idmap_text_end);
			dcache_clean_range(__hyp_text_start, __hyp_text_end);
		}

		/* make the crash dump kernel image protected again */
		crash_post_resume();
