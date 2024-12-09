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