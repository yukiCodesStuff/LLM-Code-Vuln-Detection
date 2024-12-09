	return HYBRID_INTEL_CORE;
}

/*
 * Broadwell:
 *
 * The INST_RETIRED.ALL period always needs to have lowest 6 bits cleared
 */
static void bdw_limit_period(struct perf_event *event, s64 *left)
{
	if ((event->hw.config & INTEL_ARCH_EVENT_MASK) ==
			X86_CONFIG(.event=0xc0, .umask=0x01)) {
		if (*left < 128)
			*left = 128;
		*left &= ~0x3fULL;
	}

		x86_pmu.hw_config = hsw_hw_config;
		x86_pmu.get_event_constraints = hsw_get_event_constraints;
		x86_pmu.lbr_double_abort = true;
		extra_attr = boot_cpu_has(X86_FEATURE_RTM) ?
			hsw_format_attr : nhm_format_attr;
		td_attr  = hsw_events_attrs;