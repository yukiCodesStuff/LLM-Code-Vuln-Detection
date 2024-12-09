};

struct event_constraint intel_slm_pebs_event_constraints[] = {
	/* INST_RETIRED.ANY_P, inv=1, cmask=16 (cycles:p). */
	INTEL_FLAGS_EVENT_CONSTRAINT(0x108000c0, 0x1),
	/* Allow all events as PEBS with no flags */
	INTEL_ALL_EVENT_CONSTRAINT(0, 0x1),
	EVENT_CONSTRAINT_END
};