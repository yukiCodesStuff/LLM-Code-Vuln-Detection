	bool pvclock_set_guest_stopped_request;

	struct {
		u8 preempted;
		u64 msr_val;
		u64 last_steal;
		struct gfn_to_pfn_cache cache;
	} st;

	u64 tsc_offset;