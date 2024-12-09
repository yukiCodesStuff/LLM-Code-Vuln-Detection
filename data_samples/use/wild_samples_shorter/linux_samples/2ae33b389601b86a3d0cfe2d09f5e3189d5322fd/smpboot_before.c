	unsigned int eax, ebx, ecx, edx;
	unsigned int highest_cstate = 0;
	unsigned int highest_subcstate = 0;
	int i;
	void *mwait_ptr;
	struct cpuinfo_x86 *c = __this_cpu_ptr(&cpu_info);

	if (!this_cpu_has(X86_FEATURE_MWAIT))
		return;
	if (!this_cpu_has(X86_FEATURE_CLFLSH))