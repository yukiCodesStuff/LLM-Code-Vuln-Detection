struct event_constraint intel_atom_pebs_event_constraints[] = {
	INTEL_FLAGS_UEVENT_CONSTRAINT(0x00c0, 0x1), /* INST_RETIRED.ANY */
	INTEL_FLAGS_UEVENT_CONSTRAINT(0x00c5, 0x1), /* MISPREDICTED_BRANCH_RETIRED */
	INTEL_FLAGS_EVENT_CONSTRAINT(0xcb, 0x1),    /* MEM_LOAD_RETIRED.* */
	EVENT_CONSTRAINT_END
};

struct event_constraint intel_slm_pebs_event_constraints[] = {
	/* INST_RETIRED.ANY_P, inv=1, cmask=16 (cycles:p). */
	INTEL_FLAGS_EVENT_CONSTRAINT(0x108000c0, 0x1),
	/* Allow all events as PEBS with no flags */
	INTEL_ALL_EVENT_CONSTRAINT(0, 0x1),
	EVENT_CONSTRAINT_END
};

struct event_constraint intel_nehalem_pebs_event_constraints[] = {
	INTEL_PLD_CONSTRAINT(0x100b, 0xf),      /* MEM_INST_RETIRED.* */
	INTEL_FLAGS_EVENT_CONSTRAINT(0x0f, 0xf),    /* MEM_UNCORE_RETIRED.* */
	INTEL_FLAGS_UEVENT_CONSTRAINT(0x010c, 0xf), /* MEM_STORE_RETIRED.DTLB_MISS */
	INTEL_FLAGS_EVENT_CONSTRAINT(0xc0, 0xf),    /* INST_RETIRED.ANY */
	INTEL_EVENT_CONSTRAINT(0xc2, 0xf),    /* UOPS_RETIRED.* */
	INTEL_FLAGS_EVENT_CONSTRAINT(0xc4, 0xf),    /* BR_INST_RETIRED.* */
	INTEL_FLAGS_UEVENT_CONSTRAINT(0x02c5, 0xf), /* BR_MISP_RETIRED.NEAR_CALL */
	INTEL_FLAGS_EVENT_CONSTRAINT(0xc7, 0xf),    /* SSEX_UOPS_RETIRED.* */
	INTEL_FLAGS_UEVENT_CONSTRAINT(0x20c8, 0xf), /* ITLB_MISS_RETIRED */
	INTEL_FLAGS_EVENT_CONSTRAINT(0xcb, 0xf),    /* MEM_LOAD_RETIRED.* */
	INTEL_FLAGS_EVENT_CONSTRAINT(0xf7, 0xf),    /* FP_ASSIST.* */
	EVENT_CONSTRAINT_END
};

struct event_constraint intel_westmere_pebs_event_constraints[] = {
	INTEL_PLD_CONSTRAINT(0x100b, 0xf),      /* MEM_INST_RETIRED.* */
	INTEL_FLAGS_EVENT_CONSTRAINT(0x0f, 0xf),    /* MEM_UNCORE_RETIRED.* */
	INTEL_FLAGS_UEVENT_CONSTRAINT(0x010c, 0xf), /* MEM_STORE_RETIRED.DTLB_MISS */
	INTEL_FLAGS_EVENT_CONSTRAINT(0xc0, 0xf),    /* INSTR_RETIRED.* */
	INTEL_EVENT_CONSTRAINT(0xc2, 0xf),    /* UOPS_RETIRED.* */
	INTEL_FLAGS_EVENT_CONSTRAINT(0xc4, 0xf),    /* BR_INST_RETIRED.* */
	INTEL_FLAGS_EVENT_CONSTRAINT(0xc5, 0xf),    /* BR_MISP_RETIRED.* */
	INTEL_FLAGS_EVENT_CONSTRAINT(0xc7, 0xf),    /* SSEX_UOPS_RETIRED.* */
	INTEL_FLAGS_UEVENT_CONSTRAINT(0x20c8, 0xf), /* ITLB_MISS_RETIRED */
	INTEL_FLAGS_EVENT_CONSTRAINT(0xcb, 0xf),    /* MEM_LOAD_RETIRED.* */
	INTEL_FLAGS_EVENT_CONSTRAINT(0xf7, 0xf),    /* FP_ASSIST.* */
	EVENT_CONSTRAINT_END
};

struct event_constraint intel_snb_pebs_event_constraints[] = {
	INTEL_FLAGS_UEVENT_CONSTRAINT(0x01c0, 0x2), /* INST_RETIRED.PRECDIST */
	INTEL_PLD_CONSTRAINT(0x01cd, 0x8),    /* MEM_TRANS_RETIRED.LAT_ABOVE_THR */
	INTEL_PST_CONSTRAINT(0x02cd, 0x8),    /* MEM_TRANS_RETIRED.PRECISE_STORES */
	/* UOPS_RETIRED.ALL, inv=1, cmask=16 (cycles:p). */
	INTEL_FLAGS_EVENT_CONSTRAINT(0x108001c2, 0xf),
	/* Allow all events as PEBS with no flags */
	INTEL_ALL_EVENT_CONSTRAINT(0, 0xf),
	EVENT_CONSTRAINT_END
};

struct event_constraint intel_ivb_pebs_event_constraints[] = {
        INTEL_FLAGS_UEVENT_CONSTRAINT(0x01c0, 0x2), /* INST_RETIRED.PRECDIST */
        INTEL_PLD_CONSTRAINT(0x01cd, 0x8),    /* MEM_TRANS_RETIRED.LAT_ABOVE_THR */
	INTEL_PST_CONSTRAINT(0x02cd, 0x8),    /* MEM_TRANS_RETIRED.PRECISE_STORES */
	/* UOPS_RETIRED.ALL, inv=1, cmask=16 (cycles:p). */
	INTEL_FLAGS_EVENT_CONSTRAINT(0x108001c2, 0xf),
	/* Allow all events as PEBS with no flags */
	INTEL_ALL_EVENT_CONSTRAINT(0, 0xf),
        EVENT_CONSTRAINT_END
};

struct event_constraint intel_hsw_pebs_event_constraints[] = {
	INTEL_FLAGS_UEVENT_CONSTRAINT(0x01c0, 0x2), /* INST_RETIRED.PRECDIST */
	INTEL_PLD_CONSTRAINT(0x01cd, 0xf),    /* MEM_TRANS_RETIRED.* */
	/* UOPS_RETIRED.ALL, inv=1, cmask=16 (cycles:p). */
	INTEL_FLAGS_EVENT_CONSTRAINT(0x108001c2, 0xf),
	INTEL_FLAGS_UEVENT_CONSTRAINT_DATALA_NA(0x01c2, 0xf), /* UOPS_RETIRED.ALL */
	INTEL_FLAGS_UEVENT_CONSTRAINT_DATALA_LD(0x11d0, 0xf), /* MEM_UOPS_RETIRED.STLB_MISS_LOADS */
	INTEL_FLAGS_UEVENT_CONSTRAINT_DATALA_LD(0x21d0, 0xf), /* MEM_UOPS_RETIRED.LOCK_LOADS */
	INTEL_FLAGS_UEVENT_CONSTRAINT_DATALA_LD(0x41d0, 0xf), /* MEM_UOPS_RETIRED.SPLIT_LOADS */
	INTEL_FLAGS_UEVENT_CONSTRAINT_DATALA_LD(0x81d0, 0xf), /* MEM_UOPS_RETIRED.ALL_LOADS */
	INTEL_FLAGS_UEVENT_CONSTRAINT_DATALA_ST(0x12d0, 0xf), /* MEM_UOPS_RETIRED.STLB_MISS_STORES */
	INTEL_FLAGS_UEVENT_CONSTRAINT_DATALA_ST(0x42d0, 0xf), /* MEM_UOPS_RETIRED.SPLIT_STORES */
	INTEL_FLAGS_UEVENT_CONSTRAINT_DATALA_ST(0x82d0, 0xf), /* MEM_UOPS_RETIRED.ALL_STORES */
	INTEL_FLAGS_EVENT_CONSTRAINT_DATALA_LD(0xd1, 0xf),    /* MEM_LOAD_UOPS_RETIRED.* */
	INTEL_FLAGS_EVENT_CONSTRAINT_DATALA_LD(0xd2, 0xf),    /* MEM_LOAD_UOPS_L3_HIT_RETIRED.* */
	INTEL_FLAGS_EVENT_CONSTRAINT_DATALA_LD(0xd3, 0xf),    /* MEM_LOAD_UOPS_L3_MISS_RETIRED.* */
	/* Allow all events as PEBS with no flags */
	INTEL_ALL_EVENT_CONSTRAINT(0, 0xf),
	EVENT_CONSTRAINT_END
};

struct event_constraint *intel_pebs_constraints(struct perf_event *event)
{
	struct event_constraint *c;

	if (!event->attr.precise_ip)
		return NULL;

	if (x86_pmu.pebs_constraints) {
		for_each_event_constraint(c, x86_pmu.pebs_constraints) {
			if ((event->hw.config & c->cmask) == c->code) {
				event->hw.flags |= c->flags;
				return c;
			}
		}
	}

	return &emptyconstraint;
}

void intel_pmu_pebs_enable(struct perf_event *event)
{
	struct cpu_hw_events *cpuc = this_cpu_ptr(&cpu_hw_events);
	struct hw_perf_event *hwc = &event->hw;

	hwc->config &= ~ARCH_PERFMON_EVENTSEL_INT;

	cpuc->pebs_enabled |= 1ULL << hwc->idx;

	if (event->hw.flags & PERF_X86_EVENT_PEBS_LDLAT)
		cpuc->pebs_enabled |= 1ULL << (hwc->idx + 32);
	else if (event->hw.flags & PERF_X86_EVENT_PEBS_ST)
		cpuc->pebs_enabled |= 1ULL << 63;
}

void intel_pmu_pebs_disable(struct perf_event *event)
{
	struct cpu_hw_events *cpuc = this_cpu_ptr(&cpu_hw_events);
	struct hw_perf_event *hwc = &event->hw;

	cpuc->pebs_enabled &= ~(1ULL << hwc->idx);

	if (event->hw.constraint->flags & PERF_X86_EVENT_PEBS_LDLAT)
		cpuc->pebs_enabled &= ~(1ULL << (hwc->idx + 32));
	else if (event->hw.constraint->flags & PERF_X86_EVENT_PEBS_ST)
		cpuc->pebs_enabled &= ~(1ULL << 63);

	if (cpuc->enabled)
		wrmsrl(MSR_IA32_PEBS_ENABLE, cpuc->pebs_enabled);

	hwc->config |= ARCH_PERFMON_EVENTSEL_INT;
}

void intel_pmu_pebs_enable_all(void)
{
	struct cpu_hw_events *cpuc = this_cpu_ptr(&cpu_hw_events);

	if (cpuc->pebs_enabled)
		wrmsrl(MSR_IA32_PEBS_ENABLE, cpuc->pebs_enabled);
}

void intel_pmu_pebs_disable_all(void)
{
	struct cpu_hw_events *cpuc = this_cpu_ptr(&cpu_hw_events);

	if (cpuc->pebs_enabled)
		wrmsrl(MSR_IA32_PEBS_ENABLE, 0);
}

static int intel_pmu_pebs_fixup_ip(struct pt_regs *regs)
{
	struct cpu_hw_events *cpuc = this_cpu_ptr(&cpu_hw_events);
	unsigned long from = cpuc->lbr_entries[0].from;
	unsigned long old_to, to = cpuc->lbr_entries[0].to;
	unsigned long ip = regs->ip;
	int is_64bit = 0;
	void *kaddr;
	int size;

	/*
	 * We don't need to fixup if the PEBS assist is fault like
	 */
	if (!x86_pmu.intel_cap.pebs_trap)
		return 1;

	/*
	 * No LBR entry, no basic block, no rewinding
	 */
	if (!cpuc->lbr_stack.nr || !from || !to)
		return 0;

	/*
	 * Basic blocks should never cross user/kernel boundaries
	 */
	if (kernel_ip(ip) != kernel_ip(to))
		return 0;

	/*
	 * unsigned math, either ip is before the start (impossible) or
	 * the basic block is larger than 1 page (sanity)
	 */
	if ((ip - to) > PEBS_FIXUP_SIZE)
		return 0;

	/*
	 * We sampled a branch insn, rewind using the LBR stack
	 */
	if (ip == to) {
		set_linear_ip(regs, from);
		return 1;
	}

	size = ip - to;
	if (!kernel_ip(ip)) {
		int bytes;
		u8 *buf = this_cpu_read(insn_buffer);

		/* 'size' must fit our buffer, see above */
		bytes = copy_from_user_nmi(buf, (void __user *)to, size);
		if (bytes != 0)
			return 0;

		kaddr = buf;
	} else {
		kaddr = (void *)to;
	}

	do {
		struct insn insn;

		old_to = to;

#ifdef CONFIG_X86_64
		is_64bit = kernel_ip(to) || !test_thread_flag(TIF_IA32);
#endif
		insn_init(&insn, kaddr, size, is_64bit);
		insn_get_length(&insn);
		/*
		 * Make sure there was not a problem decoding the
		 * instruction and getting the length.  This is
		 * doubly important because we have an infinite
		 * loop if insn.length=0.
		 */
		if (!insn.length)
			break;

		to += insn.length;
		kaddr += insn.length;
		size -= insn.length;
	} while (to < ip);

	if (to == ip) {
		set_linear_ip(regs, old_to);
		return 1;
	}

	/*
	 * Even though we decoded the basic block, the instruction stream
	 * never matched the given IP, either the TO or the IP got corrupted.
	 */
	return 0;
}

static inline u64 intel_hsw_weight(struct pebs_record_hsw *pebs)
{
	if (pebs->tsx_tuning) {
		union hsw_tsx_tuning tsx = { .value = pebs->tsx_tuning };
		return tsx.cycles_last_block;
	}
	return 0;
}

static inline u64 intel_hsw_transaction(struct pebs_record_hsw *pebs)
{
	u64 txn = (pebs->tsx_tuning & PEBS_HSW_TSX_FLAGS) >> 32;

	/* For RTM XABORTs also log the abort code from AX */
	if ((txn & PERF_TXN_TRANSACTION) && (pebs->ax & 1))
		txn |= ((pebs->ax >> 24) & 0xff) << PERF_TXN_ABORT_SHIFT;
	return txn;
}

static void __intel_pmu_pebs_event(struct perf_event *event,
				   struct pt_regs *iregs, void *__pebs)
{
#define PERF_X86_EVENT_PEBS_HSW_PREC \
		(PERF_X86_EVENT_PEBS_ST_HSW | \
		 PERF_X86_EVENT_PEBS_LD_HSW | \
		 PERF_X86_EVENT_PEBS_NA_HSW)
	/*
	 * We cast to the biggest pebs_record but are careful not to
	 * unconditionally access the 'extra' entries.
	 */
	struct cpu_hw_events *cpuc = this_cpu_ptr(&cpu_hw_events);
	struct pebs_record_hsw *pebs = __pebs;
	struct perf_sample_data data;
	struct pt_regs regs;
	u64 sample_type;
	int fll, fst, dsrc;
	int fl = event->hw.flags;

	if (!intel_pmu_save_and_restart(event))
		return;

	sample_type = event->attr.sample_type;
	dsrc = sample_type & PERF_SAMPLE_DATA_SRC;

	fll = fl & PERF_X86_EVENT_PEBS_LDLAT;
	fst = fl & (PERF_X86_EVENT_PEBS_ST | PERF_X86_EVENT_PEBS_HSW_PREC);

	perf_sample_data_init(&data, 0, event->hw.last_period);

	data.period = event->hw.last_period;

	/*
	 * Use latency for weight (only avail with PEBS-LL)
	 */
	if (fll && (sample_type & PERF_SAMPLE_WEIGHT))
		data.weight = pebs->lat;

	/*
	 * data.data_src encodes the data source
	 */
	if (dsrc) {
		u64 val = PERF_MEM_NA;
		if (fll)
			val = load_latency_data(pebs->dse);
		else if (fst && (fl & PERF_X86_EVENT_PEBS_HSW_PREC))
			val = precise_datala_hsw(event, pebs->dse);
		else if (fst)
			val = precise_store_data(pebs->dse);
		data.data_src.val = val;
	}

	/*
	 * We use the interrupt regs as a base because the PEBS record
	 * does not contain a full regs set, specifically it seems to
	 * lack segment descriptors, which get used by things like
	 * user_mode().
	 *
	 * In the simple case fix up only the IP and BP,SP regs, for
	 * PERF_SAMPLE_IP and PERF_SAMPLE_CALLCHAIN to function properly.
	 * A possible PERF_SAMPLE_REGS will have to transfer all regs.
	 */
	regs = *iregs;
	regs.flags = pebs->flags;
	set_linear_ip(&regs, pebs->ip);
	regs.bp = pebs->bp;
	regs.sp = pebs->sp;

	if (sample_type & PERF_SAMPLE_REGS_INTR) {
		regs.ax = pebs->ax;
		regs.bx = pebs->bx;
		regs.cx = pebs->cx;
		regs.dx = pebs->dx;
		regs.si = pebs->si;
		regs.di = pebs->di;
		regs.bp = pebs->bp;
		regs.sp = pebs->sp;

		regs.flags = pebs->flags;
#ifndef CONFIG_X86_32
		regs.r8 = pebs->r8;
		regs.r9 = pebs->r9;
		regs.r10 = pebs->r10;
		regs.r11 = pebs->r11;
		regs.r12 = pebs->r12;
		regs.r13 = pebs->r13;
		regs.r14 = pebs->r14;
		regs.r15 = pebs->r15;
#endif
	}

	if (event->attr.precise_ip > 1 && x86_pmu.intel_cap.pebs_format >= 2) {
		regs.ip = pebs->real_ip;
		regs.flags |= PERF_EFLAGS_EXACT;
	} else if (event->attr.precise_ip > 1 && intel_pmu_pebs_fixup_ip(&regs))
		regs.flags |= PERF_EFLAGS_EXACT;
	else
		regs.flags &= ~PERF_EFLAGS_EXACT;

	if ((sample_type & PERF_SAMPLE_ADDR) &&
	    x86_pmu.intel_cap.pebs_format >= 1)
		data.addr = pebs->dla;

	if (x86_pmu.intel_cap.pebs_format >= 2) {
		/* Only set the TSX weight when no memory weight. */
		if ((sample_type & PERF_SAMPLE_WEIGHT) && !fll)
			data.weight = intel_hsw_weight(pebs);

		if (sample_type & PERF_SAMPLE_TRANSACTION)
			data.txn = intel_hsw_transaction(pebs);
	}

	if (has_branch_stack(event))
		data.br_stack = &cpuc->lbr_stack;

	if (perf_event_overflow(event, &data, &regs))
		x86_pmu_stop(event, 0);
}

static void intel_pmu_drain_pebs_core(struct pt_regs *iregs)
{
	struct cpu_hw_events *cpuc = this_cpu_ptr(&cpu_hw_events);
	struct debug_store *ds = cpuc->ds;
	struct perf_event *event = cpuc->events[0]; /* PMC0 only */
	struct pebs_record_core *at, *top;
	int n;

	if (!x86_pmu.pebs_active)
		return;

	at  = (struct pebs_record_core *)(unsigned long)ds->pebs_buffer_base;
	top = (struct pebs_record_core *)(unsigned long)ds->pebs_index;

	/*
	 * Whatever else happens, drain the thing
	 */
	ds->pebs_index = ds->pebs_buffer_base;

	if (!test_bit(0, cpuc->active_mask))
		return;

	WARN_ON_ONCE(!event);

	if (!event->attr.precise_ip)
		return;

	n = top - at;
	if (n <= 0)
		return;

	/*
	 * Should not happen, we program the threshold at 1 and do not
	 * set a reset value.
	 */
	WARN_ONCE(n > 1, "bad leftover pebs %d\n", n);
	at += n - 1;

	__intel_pmu_pebs_event(event, iregs, at);
}

static void intel_pmu_drain_pebs_nhm(struct pt_regs *iregs)
{
	struct cpu_hw_events *cpuc = this_cpu_ptr(&cpu_hw_events);
	struct debug_store *ds = cpuc->ds;
	struct perf_event *event = NULL;
	void *at, *top;
	u64 status = 0;
	int bit;

	if (!x86_pmu.pebs_active)
		return;

	at  = (struct pebs_record_nhm *)(unsigned long)ds->pebs_buffer_base;
	top = (struct pebs_record_nhm *)(unsigned long)ds->pebs_index;

	ds->pebs_index = ds->pebs_buffer_base;

	if (unlikely(at > top))
		return;

	/*
	 * Should not happen, we program the threshold at 1 and do not
	 * set a reset value.
	 */
	WARN_ONCE(top - at > x86_pmu.max_pebs_events * x86_pmu.pebs_record_size,
		  "Unexpected number of pebs records %ld\n",
		  (long)(top - at) / x86_pmu.pebs_record_size);

	for (; at < top; at += x86_pmu.pebs_record_size) {
		struct pebs_record_nhm *p = at;

		for_each_set_bit(bit, (unsigned long *)&p->status,
				 x86_pmu.max_pebs_events) {
			event = cpuc->events[bit];
			if (!test_bit(bit, cpuc->active_mask))
				continue;

			WARN_ON_ONCE(!event);

			if (!event->attr.precise_ip)
				continue;

			if (__test_and_set_bit(bit, (unsigned long *)&status))
				continue;

			break;
		}

		if (!event || bit >= x86_pmu.max_pebs_events)
			continue;

		__intel_pmu_pebs_event(event, iregs, at);
	}
}

/*
 * BTS, PEBS probe and setup
 */

void __init intel_ds_init(void)
{
	/*
	 * No support for 32bit formats
	 */
	if (!boot_cpu_has(X86_FEATURE_DTES64))
		return;

	x86_pmu.bts  = boot_cpu_has(X86_FEATURE_BTS);
	x86_pmu.pebs = boot_cpu_has(X86_FEATURE_PEBS);
	if (x86_pmu.pebs) {
		char pebs_type = x86_pmu.intel_cap.pebs_trap ?  '+' : '-';
		int format = x86_pmu.intel_cap.pebs_format;

		switch (format) {
		case 0:
			printk(KERN_CONT "PEBS fmt0%c, ", pebs_type);
			x86_pmu.pebs_record_size = sizeof(struct pebs_record_core);
			x86_pmu.drain_pebs = intel_pmu_drain_pebs_core;
			break;

		case 1:
			printk(KERN_CONT "PEBS fmt1%c, ", pebs_type);
			x86_pmu.pebs_record_size = sizeof(struct pebs_record_nhm);
			x86_pmu.drain_pebs = intel_pmu_drain_pebs_nhm;
			break;

		case 2:
			pr_cont("PEBS fmt2%c, ", pebs_type);
			x86_pmu.pebs_record_size = sizeof(struct pebs_record_hsw);
			x86_pmu.drain_pebs = intel_pmu_drain_pebs_nhm;
			break;

		default:
			printk(KERN_CONT "no PEBS fmt%d%c, ", format, pebs_type);
			x86_pmu.pebs = 0;
		}
	}
}

void perf_restore_debug_store(void)
{
	struct debug_store *ds = __this_cpu_read(cpu_hw_events.ds);

	if (!x86_pmu.bts && !x86_pmu.pebs)
		return;

	wrmsrl(MSR_IA32_DS_AREA, (unsigned long)ds);
}