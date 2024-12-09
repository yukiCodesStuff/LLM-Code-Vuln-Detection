	return best && (best->ebx & bit(X86_FEATURE_SMEP));
}

static inline bool guest_cpuid_has_fsgsbase(struct kvm_vcpu *vcpu)
{
	struct kvm_cpuid_entry2 *best;
