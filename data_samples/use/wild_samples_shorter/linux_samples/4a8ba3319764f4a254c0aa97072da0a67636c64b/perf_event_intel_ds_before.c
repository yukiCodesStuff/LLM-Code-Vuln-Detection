};

struct event_constraint intel_slm_pebs_event_constraints[] = {
	/* UOPS_RETIRED.ALL, inv=1, cmask=16 (cycles:p). */
	INTEL_FLAGS_EVENT_CONSTRAINT(0x108001c2, 0xf),
	/* Allow all events as PEBS with no flags */
	INTEL_ALL_EVENT_CONSTRAINT(0, 0x1),
	EVENT_CONSTRAINT_END
};