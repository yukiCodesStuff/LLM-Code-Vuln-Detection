	bool pvclock_set_guest_stopped_request;

	struct {
		u64 msr_val;
		u64 last_steal;
		struct gfn_to_hva_cache stime;
		struct kvm_steal_time steal;
		struct gfn_to_pfn_cache cache;
	} st;

	u64 tsc_offset;