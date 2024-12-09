		tlb_lld_4m[ENTRIES], tlb_lld_1g[ENTRIES]);
}

void detect_ht(struct cpuinfo_x86 *c)
{
#ifdef CONFIG_SMP
	u32 eax, ebx, ecx, edx;
	int index_msb, core_bits;
	static bool printed;

	if (!cpu_has(c, X86_FEATURE_HT))
		return;

	if (cpu_has(c, X86_FEATURE_CMP_LEGACY))
		goto out;

	if (cpu_has(c, X86_FEATURE_XTOPOLOGY))
		return;

	cpuid(1, &eax, &ebx, &ecx, &edx);

	smp_num_siblings = (ebx & 0xff0000) >> 16;

	if (smp_num_siblings == 1) {
		pr_info_once("CPU0: Hyper-Threading is disabled\n");
		goto out;
	}

	if (smp_num_siblings <= 1)
		goto out;

	index_msb = get_count_order(smp_num_siblings);
	c->phys_proc_id = apic->phys_pkg_id(c->initial_apicid, index_msb);


	c->cpu_core_id = apic->phys_pkg_id(c->initial_apicid, index_msb) &
				       ((1 << core_bits) - 1);

out:
	if (!printed && (c->x86_max_cores * smp_num_siblings) > 1) {
		pr_info("CPU: Physical Processor ID: %d\n",
			c->phys_proc_id);
		pr_info("CPU: Processor Core ID: %d\n",
			c->cpu_core_id);
		printed = 1;
	}
#endif
}

static void get_cpu_vendor(struct cpuinfo_x86 *c)
	{}
};

static void __init cpu_set_bug_bits(struct cpuinfo_x86 *c)
{
	u64 ia32_cap = 0;

		return;

	setup_force_cpu_bug(X86_BUG_CPU_MELTDOWN);
}

/*
 * The NOPL instruction is supposed to exist on all CPUs of family >= 6;