{
	return HYBRID_INTEL_CORE;
}

/*
 * Broadwell:
 *
 * The INST_RETIRED.ALL period always needs to have lowest 6 bits cleared
 * (BDM55) and it must not use a period smaller than 100 (BDM11). We combine
 * the two to enforce a minimum period of 128 (the smallest value that has bits
 * 0-5 cleared and >= 100).
 *
 * Because of how the code in x86_perf_event_set_period() works, the truncation
 * of the lower 6 bits is 'harmless' as we'll occasionally add a longer period
 * to make up for the 'lost' events due to carrying the 'error' in period_left.
 *
 * Therefore the effective (average) period matches the requested period,
 * despite coarser hardware granularity.
 */
static void bdw_limit_period(struct perf_event *event, s64 *left)
{
	if ((event->hw.config & INTEL_ARCH_EVENT_MASK) ==
			X86_CONFIG(.event=0xc0, .umask=0x01)) {
		if (*left < 128)
			*left = 128;
		*left &= ~0x3fULL;
	}
{
	return HYBRID_INTEL_CORE;
}

/*
 * Broadwell:
 *
 * The INST_RETIRED.ALL period always needs to have lowest 6 bits cleared
 * (BDM55) and it must not use a period smaller than 100 (BDM11). We combine
 * the two to enforce a minimum period of 128 (the smallest value that has bits
 * 0-5 cleared and >= 100).
 *
 * Because of how the code in x86_perf_event_set_period() works, the truncation
 * of the lower 6 bits is 'harmless' as we'll occasionally add a longer period
 * to make up for the 'lost' events due to carrying the 'error' in period_left.
 *
 * Therefore the effective (average) period matches the requested period,
 * despite coarser hardware granularity.
 */
static void bdw_limit_period(struct perf_event *event, s64 *left)
{
	if ((event->hw.config & INTEL_ARCH_EVENT_MASK) ==
			X86_CONFIG(.event=0xc0, .umask=0x01)) {
		if (*left < 128)
			*left = 128;
		*left &= ~0x3fULL;
	}
}
	switch (boot_cpu_data.x86_vfm) {
	case INTEL_CORE_YONAH:
		pr_cont("Core events, ");
		name = "core";
		break;

	case INTEL_CORE2_MEROM:
		x86_add_quirk(intel_clovertown_quirk);
		fallthrough;

	case INTEL_CORE2_MEROM_L:
	case INTEL_CORE2_PENRYN:
	case INTEL_CORE2_DUNNINGTON:
		memcpy(hw_cache_event_ids, core2_hw_cache_event_ids,
		       sizeof(hw_cache_event_ids));

		intel_pmu_lbr_init_core();

		x86_pmu.event_constraints = intel_core2_event_constraints;
		x86_pmu.pebs_constraints = intel_core2_pebs_event_constraints;
		pr_cont("Core2 events, ");
		name = "core2";
		break;

	case INTEL_NEHALEM:
	case INTEL_NEHALEM_EP:
	case INTEL_NEHALEM_EX:
		memcpy(hw_cache_event_ids, nehalem_hw_cache_event_ids,
		       sizeof(hw_cache_event_ids));
		memcpy(hw_cache_extra_regs, nehalem_hw_cache_extra_regs,
		       sizeof(hw_cache_extra_regs));

		intel_pmu_lbr_init_nhm();

		x86_pmu.event_constraints = intel_nehalem_event_constraints;
		x86_pmu.pebs_constraints = intel_nehalem_pebs_event_constraints;
		x86_pmu.enable_all = intel_pmu_nhm_enable_all;
		x86_pmu.extra_regs = intel_nehalem_extra_regs;
		x86_pmu.limit_period = nhm_limit_period;

		mem_attr = nhm_mem_events_attrs;

		/* UOPS_ISSUED.STALLED_CYCLES */
		intel_perfmon_event_map[PERF_COUNT_HW_STALLED_CYCLES_FRONTEND] =
			X86_CONFIG(.event=0x0e, .umask=0x01, .inv=1, .cmask=1);
		/* UOPS_EXECUTED.CORE_ACTIVE_CYCLES,c=1,i=1 */
		intel_perfmon_event_map[PERF_COUNT_HW_STALLED_CYCLES_BACKEND] =
			X86_CONFIG(.event=0xb1, .umask=0x3f, .inv=1, .cmask=1);

		intel_pmu_pebs_data_source_nhm();
		x86_add_quirk(intel_nehalem_quirk);
		x86_pmu.pebs_no_tlb = 1;
		extra_attr = nhm_format_attr;

		pr_cont("Nehalem events, ");
		name = "nehalem";
		break;

	case INTEL_ATOM_BONNELL:
	case INTEL_ATOM_BONNELL_MID:
	case INTEL_ATOM_SALTWELL:
	case INTEL_ATOM_SALTWELL_MID:
	case INTEL_ATOM_SALTWELL_TABLET:
		memcpy(hw_cache_event_ids, atom_hw_cache_event_ids,
		       sizeof(hw_cache_event_ids));

		intel_pmu_lbr_init_atom();

		x86_pmu.event_constraints = intel_gen_event_constraints;
		x86_pmu.pebs_constraints = intel_atom_pebs_event_constraints;
		x86_pmu.pebs_aliases = intel_pebs_aliases_core2;
		pr_cont("Atom events, ");
		name = "bonnell";
		break;

	case INTEL_ATOM_SILVERMONT:
	case INTEL_ATOM_SILVERMONT_D:
	case INTEL_ATOM_SILVERMONT_MID:
	case INTEL_ATOM_AIRMONT:
	case INTEL_ATOM_AIRMONT_MID:
		memcpy(hw_cache_event_ids, slm_hw_cache_event_ids,
			sizeof(hw_cache_event_ids));
		memcpy(hw_cache_extra_regs, slm_hw_cache_extra_regs,
		       sizeof(hw_cache_extra_regs));

		intel_pmu_lbr_init_slm();

		x86_pmu.event_constraints = intel_slm_event_constraints;
		x86_pmu.pebs_constraints = intel_slm_pebs_event_constraints;
		x86_pmu.extra_regs = intel_slm_extra_regs;
		x86_pmu.flags |= PMU_FL_HAS_RSP_1;
		td_attr = slm_events_attrs;
		extra_attr = slm_format_attr;
		pr_cont("Silvermont events, ");
		name = "silvermont";
		break;

	case INTEL_ATOM_GOLDMONT:
	case INTEL_ATOM_GOLDMONT_D:
		memcpy(hw_cache_event_ids, glm_hw_cache_event_ids,
		       sizeof(hw_cache_event_ids));
		memcpy(hw_cache_extra_regs, glm_hw_cache_extra_regs,
		       sizeof(hw_cache_extra_regs));

		intel_pmu_lbr_init_skl();

		x86_pmu.event_constraints = intel_slm_event_constraints;
		x86_pmu.pebs_constraints = intel_glm_pebs_event_constraints;
		x86_pmu.extra_regs = intel_glm_extra_regs;
		/*
		 * It's recommended to use CPU_CLK_UNHALTED.CORE_P + NPEBS
		 * for precise cycles.
		 * :pp is identical to :ppp
		 */
		x86_pmu.pebs_aliases = NULL;
		x86_pmu.pebs_prec_dist = true;
		x86_pmu.lbr_pt_coexist = true;
		x86_pmu.flags |= PMU_FL_HAS_RSP_1;
		td_attr = glm_events_attrs;
		extra_attr = slm_format_attr;
		pr_cont("Goldmont events, ");
		name = "goldmont";
		break;

	case INTEL_ATOM_GOLDMONT_PLUS:
		memcpy(hw_cache_event_ids, glp_hw_cache_event_ids,
		       sizeof(hw_cache_event_ids));
		memcpy(hw_cache_extra_regs, glp_hw_cache_extra_regs,
		       sizeof(hw_cache_extra_regs));

		intel_pmu_lbr_init_skl();

		x86_pmu.event_constraints = intel_slm_event_constraints;
		x86_pmu.extra_regs = intel_glm_extra_regs;
		/*
		 * It's recommended to use CPU_CLK_UNHALTED.CORE_P + NPEBS
		 * for precise cycles.
		 */
		x86_pmu.pebs_aliases = NULL;
		x86_pmu.pebs_prec_dist = true;
		x86_pmu.lbr_pt_coexist = true;
		x86_pmu.pebs_capable = ~0ULL;
		x86_pmu.flags |= PMU_FL_HAS_RSP_1;
		x86_pmu.flags |= PMU_FL_PEBS_ALL;
		x86_pmu.get_event_constraints = glp_get_event_constraints;
		td_attr = glm_events_attrs;
		/* Goldmont Plus has 4-wide pipeline */
		event_attr_td_total_slots_scale_glm.event_str = "4";
		extra_attr = slm_format_attr;
		pr_cont("Goldmont plus events, ");
		name = "goldmont_plus";
		break;

	case INTEL_ATOM_TREMONT_D:
	case INTEL_ATOM_TREMONT:
	case INTEL_ATOM_TREMONT_L:
		x86_pmu.late_ack = true;
		memcpy(hw_cache_event_ids, glp_hw_cache_event_ids,
		       sizeof(hw_cache_event_ids));
		memcpy(hw_cache_extra_regs, tnt_hw_cache_extra_regs,
		       sizeof(hw_cache_extra_regs));
		hw_cache_event_ids[C(ITLB)][C(OP_READ)][C(RESULT_ACCESS)] = -1;

		intel_pmu_lbr_init_skl();

		x86_pmu.event_constraints = intel_slm_event_constraints;
		x86_pmu.extra_regs = intel_tnt_extra_regs;
		/*
		 * It's recommended to use CPU_CLK_UNHALTED.CORE_P + NPEBS
		 * for precise cycles.
		 */
		x86_pmu.pebs_aliases = NULL;
		x86_pmu.pebs_prec_dist = true;
		x86_pmu.lbr_pt_coexist = true;
		x86_pmu.flags |= PMU_FL_HAS_RSP_1;
		x86_pmu.get_event_constraints = tnt_get_event_constraints;
		td_attr = tnt_events_attrs;
		extra_attr = slm_format_attr;
		pr_cont("Tremont events, ");
		name = "Tremont";
		break;

	case INTEL_ATOM_GRACEMONT:
		intel_pmu_init_grt(NULL);
		intel_pmu_pebs_data_source_grt();
		x86_pmu.pebs_latency_data = grt_latency_data;
		x86_pmu.get_event_constraints = tnt_get_event_constraints;
		td_attr = tnt_events_attrs;
		mem_attr = grt_mem_attrs;
		extra_attr = nhm_format_attr;
		pr_cont("Gracemont events, ");
		name = "gracemont";
		break;

	case INTEL_ATOM_CRESTMONT:
	case INTEL_ATOM_CRESTMONT_X:
		intel_pmu_init_grt(NULL);
		x86_pmu.extra_regs = intel_cmt_extra_regs;
		intel_pmu_pebs_data_source_cmt();
		x86_pmu.pebs_latency_data = cmt_latency_data;
		x86_pmu.get_event_constraints = cmt_get_event_constraints;
		td_attr = cmt_events_attrs;
		mem_attr = grt_mem_attrs;
		extra_attr = cmt_format_attr;
		pr_cont("Crestmont events, ");
		name = "crestmont";
		break;

	case INTEL_WESTMERE:
	case INTEL_WESTMERE_EP:
	case INTEL_WESTMERE_EX:
		memcpy(hw_cache_event_ids, westmere_hw_cache_event_ids,
		       sizeof(hw_cache_event_ids));
		memcpy(hw_cache_extra_regs, nehalem_hw_cache_extra_regs,
		       sizeof(hw_cache_extra_regs));

		intel_pmu_lbr_init_nhm();

		x86_pmu.event_constraints = intel_westmere_event_constraints;
		x86_pmu.enable_all = intel_pmu_nhm_enable_all;
		x86_pmu.pebs_constraints = intel_westmere_pebs_event_constraints;
		x86_pmu.extra_regs = intel_westmere_extra_regs;
		x86_pmu.flags |= PMU_FL_HAS_RSP_1;

		mem_attr = nhm_mem_events_attrs;

		/* UOPS_ISSUED.STALLED_CYCLES */
		intel_perfmon_event_map[PERF_COUNT_HW_STALLED_CYCLES_FRONTEND] =
			X86_CONFIG(.event=0x0e, .umask=0x01, .inv=1, .cmask=1);
		/* UOPS_EXECUTED.CORE_ACTIVE_CYCLES,c=1,i=1 */
		intel_perfmon_event_map[PERF_COUNT_HW_STALLED_CYCLES_BACKEND] =
			X86_CONFIG(.event=0xb1, .umask=0x3f, .inv=1, .cmask=1);

		intel_pmu_pebs_data_source_nhm();
		extra_attr = nhm_format_attr;
		pr_cont("Westmere events, ");
		name = "westmere";
		break;

	case INTEL_SANDYBRIDGE:
	case INTEL_SANDYBRIDGE_X:
		x86_add_quirk(intel_sandybridge_quirk);
		x86_add_quirk(intel_ht_bug);
		memcpy(hw_cache_event_ids, snb_hw_cache_event_ids,
		       sizeof(hw_cache_event_ids));
		memcpy(hw_cache_extra_regs, snb_hw_cache_extra_regs,
		       sizeof(hw_cache_extra_regs));

		intel_pmu_lbr_init_snb();

		x86_pmu.event_constraints = intel_snb_event_constraints;
		x86_pmu.pebs_constraints = intel_snb_pebs_event_constraints;
		x86_pmu.pebs_aliases = intel_pebs_aliases_snb;
		if (boot_cpu_data.x86_vfm == INTEL_SANDYBRIDGE_X)
			x86_pmu.extra_regs = intel_snbep_extra_regs;
		else
			x86_pmu.extra_regs = intel_snb_extra_regs;


		/* all extra regs are per-cpu when HT is on */
		x86_pmu.flags |= PMU_FL_HAS_RSP_1;
		x86_pmu.flags |= PMU_FL_NO_HT_SHARING;

		td_attr  = snb_events_attrs;
		mem_attr = snb_mem_events_attrs;

		/* UOPS_ISSUED.ANY,c=1,i=1 to count stall cycles */
		intel_perfmon_event_map[PERF_COUNT_HW_STALLED_CYCLES_FRONTEND] =
			X86_CONFIG(.event=0x0e, .umask=0x01, .inv=1, .cmask=1);
		/* UOPS_DISPATCHED.THREAD,c=1,i=1 to count stall cycles*/
		intel_perfmon_event_map[PERF_COUNT_HW_STALLED_CYCLES_BACKEND] =
			X86_CONFIG(.event=0xb1, .umask=0x01, .inv=1, .cmask=1);

		extra_attr = nhm_format_attr;

		pr_cont("SandyBridge events, ");
		name = "sandybridge";
		break;

	case INTEL_IVYBRIDGE:
	case INTEL_IVYBRIDGE_X:
		x86_add_quirk(intel_ht_bug);
		memcpy(hw_cache_event_ids, snb_hw_cache_event_ids,
		       sizeof(hw_cache_event_ids));
		/* dTLB-load-misses on IVB is different than SNB */
		hw_cache_event_ids[C(DTLB)][C(OP_READ)][C(RESULT_MISS)] = 0x8108; /* DTLB_LOAD_MISSES.DEMAND_LD_MISS_CAUSES_A_WALK */

		memcpy(hw_cache_extra_regs, snb_hw_cache_extra_regs,
		       sizeof(hw_cache_extra_regs));

		intel_pmu_lbr_init_snb();

		x86_pmu.event_constraints = intel_ivb_event_constraints;
		x86_pmu.pebs_constraints = intel_ivb_pebs_event_constraints;
		x86_pmu.pebs_aliases = intel_pebs_aliases_ivb;
		x86_pmu.pebs_prec_dist = true;
		if (boot_cpu_data.x86_vfm == INTEL_IVYBRIDGE_X)
			x86_pmu.extra_regs = intel_snbep_extra_regs;
		else
			x86_pmu.extra_regs = intel_snb_extra_regs;
		/* all extra regs are per-cpu when HT is on */
		x86_pmu.flags |= PMU_FL_HAS_RSP_1;
		x86_pmu.flags |= PMU_FL_NO_HT_SHARING;

		td_attr  = snb_events_attrs;
		mem_attr = snb_mem_events_attrs;

		/* UOPS_ISSUED.ANY,c=1,i=1 to count stall cycles */
		intel_perfmon_event_map[PERF_COUNT_HW_STALLED_CYCLES_FRONTEND] =
			X86_CONFIG(.event=0x0e, .umask=0x01, .inv=1, .cmask=1);

		extra_attr = nhm_format_attr;

		pr_cont("IvyBridge events, ");
		name = "ivybridge";
		break;


	case INTEL_HASWELL:
	case INTEL_HASWELL_X:
	case INTEL_HASWELL_L:
	case INTEL_HASWELL_G:
		x86_add_quirk(intel_ht_bug);
		x86_add_quirk(intel_pebs_isolation_quirk);
		x86_pmu.late_ack = true;
		memcpy(hw_cache_event_ids, hsw_hw_cache_event_ids, sizeof(hw_cache_event_ids));
		memcpy(hw_cache_extra_regs, hsw_hw_cache_extra_regs, sizeof(hw_cache_extra_regs));

		intel_pmu_lbr_init_hsw();

		x86_pmu.event_constraints = intel_hsw_event_constraints;
		x86_pmu.pebs_constraints = intel_hsw_pebs_event_constraints;
		x86_pmu.extra_regs = intel_snbep_extra_regs;
		x86_pmu.pebs_aliases = intel_pebs_aliases_ivb;
		x86_pmu.pebs_prec_dist = true;
		/* all extra regs are per-cpu when HT is on */
		x86_pmu.flags |= PMU_FL_HAS_RSP_1;
		x86_pmu.flags |= PMU_FL_NO_HT_SHARING;

		x86_pmu.hw_config = hsw_hw_config;
		x86_pmu.get_event_constraints = hsw_get_event_constraints;
		x86_pmu.lbr_double_abort = true;
		extra_attr = boot_cpu_has(X86_FEATURE_RTM) ?
			hsw_format_attr : nhm_format_attr;
		td_attr  = hsw_events_attrs;
		mem_attr = hsw_mem_events_attrs;
		tsx_attr = hsw_tsx_events_attrs;
		pr_cont("Haswell events, ");
		name = "haswell";
		break;

	case INTEL_BROADWELL:
	case INTEL_BROADWELL_D:
	case INTEL_BROADWELL_G:
	case INTEL_BROADWELL_X:
		x86_add_quirk(intel_pebs_isolation_quirk);
		x86_pmu.late_ack = true;
		memcpy(hw_cache_event_ids, hsw_hw_cache_event_ids, sizeof(hw_cache_event_ids));
		memcpy(hw_cache_extra_regs, hsw_hw_cache_extra_regs, sizeof(hw_cache_extra_regs));

		/* L3_MISS_LOCAL_DRAM is BIT(26) in Broadwell */
		hw_cache_extra_regs[C(LL)][C(OP_READ)][C(RESULT_MISS)] = HSW_DEMAND_READ |
									 BDW_L3_MISS|HSW_SNOOP_DRAM;
		hw_cache_extra_regs[C(LL)][C(OP_WRITE)][C(RESULT_MISS)] = HSW_DEMAND_WRITE|BDW_L3_MISS|
									  HSW_SNOOP_DRAM;
		hw_cache_extra_regs[C(NODE)][C(OP_READ)][C(RESULT_ACCESS)] = HSW_DEMAND_READ|
									     BDW_L3_MISS_LOCAL|HSW_SNOOP_DRAM;
		hw_cache_extra_regs[C(NODE)][C(OP_WRITE)][C(RESULT_ACCESS)] = HSW_DEMAND_WRITE|
									      BDW_L3_MISS_LOCAL|HSW_SNOOP_DRAM;

		intel_pmu_lbr_init_hsw();

		x86_pmu.event_constraints = intel_bdw_event_constraints;
		x86_pmu.pebs_constraints = intel_bdw_pebs_event_constraints;
		x86_pmu.extra_regs = intel_snbep_extra_regs;
		x86_pmu.pebs_aliases = intel_pebs_aliases_ivb;
		x86_pmu.pebs_prec_dist = true;
		/* all extra regs are per-cpu when HT is on */
		x86_pmu.flags |= PMU_FL_HAS_RSP_1;
		x86_pmu.flags |= PMU_FL_NO_HT_SHARING;

		x86_pmu.hw_config = hsw_hw_config;
		x86_pmu.get_event_constraints = hsw_get_event_constraints;
		x86_pmu.limit_period = bdw_limit_period;
		extra_attr = boot_cpu_has(X86_FEATURE_RTM) ?
			hsw_format_attr : nhm_format_attr;
		td_attr  = hsw_events_attrs;
		mem_attr = hsw_mem_events_attrs;
		tsx_attr = hsw_tsx_events_attrs;
		pr_cont("Broadwell events, ");
		name = "broadwell";
		break;

	case INTEL_XEON_PHI_KNL:
	case INTEL_XEON_PHI_KNM:
		memcpy(hw_cache_event_ids,
		       slm_hw_cache_event_ids, sizeof(hw_cache_event_ids));
		memcpy(hw_cache_extra_regs,
		       knl_hw_cache_extra_regs, sizeof(hw_cache_extra_regs));
		intel_pmu_lbr_init_knl();

		x86_pmu.event_constraints = intel_slm_event_constraints;
		x86_pmu.pebs_constraints = intel_slm_pebs_event_constraints;
		x86_pmu.extra_regs = intel_knl_extra_regs;

		/* all extra regs are per-cpu when HT is on */
		x86_pmu.flags |= PMU_FL_HAS_RSP_1;
		x86_pmu.flags |= PMU_FL_NO_HT_SHARING;
		extra_attr = slm_format_attr;
		pr_cont("Knights Landing/Mill events, ");
		name = "knights-landing";
		break;

	case INTEL_SKYLAKE_X:
		pmem = true;
		fallthrough;
	case INTEL_SKYLAKE_L:
	case INTEL_SKYLAKE:
	case INTEL_KABYLAKE_L:
	case INTEL_KABYLAKE:
	case INTEL_COMETLAKE_L:
	case INTEL_COMETLAKE:
		x86_add_quirk(intel_pebs_isolation_quirk);
		x86_pmu.late_ack = true;
		memcpy(hw_cache_event_ids, skl_hw_cache_event_ids, sizeof(hw_cache_event_ids));
		memcpy(hw_cache_extra_regs, skl_hw_cache_extra_regs, sizeof(hw_cache_extra_regs));
		intel_pmu_lbr_init_skl();

		/* INT_MISC.RECOVERY_CYCLES has umask 1 in Skylake */
		event_attr_td_recovery_bubbles.event_str_noht =
			"event=0xd,umask=0x1,cmask=1";
		event_attr_td_recovery_bubbles.event_str_ht =
			"event=0xd,umask=0x1,cmask=1,any=1";

		x86_pmu.event_constraints = intel_skl_event_constraints;
		x86_pmu.pebs_constraints = intel_skl_pebs_event_constraints;
		x86_pmu.extra_regs = intel_skl_extra_regs;
		x86_pmu.pebs_aliases = intel_pebs_aliases_skl;
		x86_pmu.pebs_prec_dist = true;
		/* all extra regs are per-cpu when HT is on */
		x86_pmu.flags |= PMU_FL_HAS_RSP_1;
		x86_pmu.flags |= PMU_FL_NO_HT_SHARING;

		x86_pmu.hw_config = hsw_hw_config;
		x86_pmu.get_event_constraints = hsw_get_event_constraints;
		extra_attr = boot_cpu_has(X86_FEATURE_RTM) ?
			hsw_format_attr : nhm_format_attr;
		extra_skl_attr = skl_format_attr;
		td_attr  = hsw_events_attrs;
		mem_attr = hsw_mem_events_attrs;
		tsx_attr = hsw_tsx_events_attrs;
		intel_pmu_pebs_data_source_skl(pmem);

		/*
		 * Processors with CPUID.RTM_ALWAYS_ABORT have TSX deprecated by default.
		 * TSX force abort hooks are not required on these systems. Only deploy
		 * workaround when microcode has not enabled X86_FEATURE_RTM_ALWAYS_ABORT.
		 */
		if (boot_cpu_has(X86_FEATURE_TSX_FORCE_ABORT) &&
		   !boot_cpu_has(X86_FEATURE_RTM_ALWAYS_ABORT)) {
			x86_pmu.flags |= PMU_FL_TFA;
			x86_pmu.get_event_constraints = tfa_get_event_constraints;
			x86_pmu.enable_all = intel_tfa_pmu_enable_all;
			x86_pmu.commit_scheduling = intel_tfa_commit_scheduling;
		}