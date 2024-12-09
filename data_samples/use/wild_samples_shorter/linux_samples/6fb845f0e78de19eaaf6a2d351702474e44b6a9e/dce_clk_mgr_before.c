			dc,
			context->bw.dce.sclk_khz);

	pp_display_cfg->min_dcfclock_khz = pp_display_cfg->min_engine_clock_khz;

	pp_display_cfg->min_engine_clock_deep_sleep_khz
			= context->bw.dce.sclk_deep_sleep_khz;
