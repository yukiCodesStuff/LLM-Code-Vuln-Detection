			dc,
			context->bw.dce.sclk_khz);

	/*
	 * As workaround for >4x4K lightup set dcfclock to min_engine_clock value.
	 * This is not required for less than 5 displays,
	 * thus don't request decfclk in dc to avoid impact
	 * on power saving.
	 *
	 */
	pp_display_cfg->min_dcfclock_khz = (context->stream_count > 4)?
			pp_display_cfg->min_engine_clock_khz : 0;

	pp_display_cfg->min_engine_clock_deep_sleep_khz
			= context->bw.dce.sclk_deep_sleep_khz;
