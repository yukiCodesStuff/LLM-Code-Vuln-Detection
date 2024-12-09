	return HYBRID_INTEL_CORE;
}

static inline bool erratum_hsw11(struct perf_event *event)
{
	return (event->hw.config & INTEL_ARCH_EVENT_MASK) ==
		X86_CONFIG(.event=0xc0, .umask=0x01);
}

/*
 * The HSW11 requires a period larger than 100 which is the same as the BDM11.
 * A minimum period of 128 is enforced as well for the INST_RETIRED.ALL.
 *
 * The message 'interrupt took too long' can be observed on any counter which
 * was armed with a period < 32 and two events expired in the same NMI.
 * A minimum period of 32 is enforced for the rest of the events.
 */
static void hsw_limit_period(struct perf_event *event, s64 *left)
{
	*left = max(*left, erratum_hsw11(event) ? 128 : 32);
}

/*
 * Broadwell:
 *
 * The INST_RETIRED.ALL period always needs to have lowest 6 bits cleared
 */
static void bdw_limit_period(struct perf_event *event, s64 *left)
{
	if (erratum_hsw11(event)) {
		if (*left < 128)
			*left = 128;
		*left &= ~0x3fULL;
	}

		x86_pmu.hw_config = hsw_hw_config;
		x86_pmu.get_event_constraints = hsw_get_event_constraints;
		x86_pmu.limit_period = hsw_limit_period;
		x86_pmu.lbr_double_abort = true;
		extra_attr = boot_cpu_has(X86_FEATURE_RTM) ?
			hsw_format_attr : nhm_format_attr;
		td_attr  = hsw_events_attrs;