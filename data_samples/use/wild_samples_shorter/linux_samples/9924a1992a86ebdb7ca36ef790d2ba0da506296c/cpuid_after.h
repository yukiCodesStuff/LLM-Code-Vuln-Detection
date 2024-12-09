{
	struct kvm_cpuid_entry2 *best;

	if (!static_cpu_has(X86_FEATURE_XSAVE))
		return 0;

	best = kvm_find_cpuid_entry(vcpu, 1, 0);
	return best && (best->ecx & bit(X86_FEATURE_XSAVE));
}
