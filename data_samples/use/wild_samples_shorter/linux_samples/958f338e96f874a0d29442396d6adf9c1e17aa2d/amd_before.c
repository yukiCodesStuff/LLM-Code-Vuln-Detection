	c->cpu_core_id %= cus_per_node;
}

/*
 * Fixup core topology information for
 * (1) AMD multi-node processors
 *     Assumption: Number of cores in each internal node is the same.
		cpuid(0x8000001e, &eax, &ebx, &ecx, &edx);

		node_id  = ecx & 0xff;
		smp_num_siblings = ((ebx >> 8) & 0xff) + 1;

		if (c->x86 == 0x15)
			c->cu_id = ebx & 0xff;


static void early_init_amd(struct cpuinfo_x86 *c)
{
	u32 dummy;

	early_init_amd_mc(c);

		set_cpu_bug(c, X86_BUG_AMD_E400);

	early_detect_mem_encrypt(c);
}

static void init_amd_k8(struct cpuinfo_x86 *c)
{
{
	u64 value;

	/* re-enable TopologyExtensions if switched off by BIOS */
	if ((c->x86_model >= 0x10) && (c->x86_model <= 0x6f) &&
	    !cpu_has(c, X86_FEATURE_TOPOEXT)) {

		if (msr_set_bit(0xc0011005, 54) > 0) {
			rdmsrl(0xc0011005, value);
			if (value & BIT_64(54)) {
				set_cpu_cap(c, X86_FEATURE_TOPOEXT);
				pr_info_once(FW_INFO "CPU: Re-enabling disabled Topology Extensions Support.\n");
			}
		}
	}

	/*
	 * The way access filter has a performance penalty on some workloads.
	 * Disable it on the affected CPUs.
	 */

	cpu_detect_cache_sizes(c);

	/* Multi core CPU? */
	if (c->extended_cpuid_level >= 0x80000008) {
		amd_detect_cmp(c);
		amd_get_topology(c);
		srat_detect_node(c);
	}

#ifdef CONFIG_X86_32
	detect_ht(c);
#endif

	init_amd_cacheinfo(c);

	if (cpu_has(c, X86_FEATURE_XMM2)) {