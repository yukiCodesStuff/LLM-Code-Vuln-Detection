{
	struct kvm_cpuid_entry2 *best;

	best = kvm_find_cpuid_entry(vcpu, 7, 0);
	return best && (best->ebx & bit(X86_FEATURE_SMEP));
}

static inline bool guest_cpuid_has_smap(struct kvm_vcpu *vcpu)
{
	struct kvm_cpuid_entry2 *best;

	best = kvm_find_cpuid_entry(vcpu, 7, 0);
	return best && (best->ebx & bit(X86_FEATURE_SMAP));
}

static inline bool guest_cpuid_has_fsgsbase(struct kvm_vcpu *vcpu)
{
	struct kvm_cpuid_entry2 *best;

	best = kvm_find_cpuid_entry(vcpu, 7, 0);
	return best && (best->ebx & bit(X86_FEATURE_FSGSBASE));
}

static inline bool guest_cpuid_has_osvw(struct kvm_vcpu *vcpu)
{
	struct kvm_cpuid_entry2 *best;

	best = kvm_find_cpuid_entry(vcpu, 0x80000001, 0);
	return best && (best->ecx & bit(X86_FEATURE_OSVW));
}

static inline bool guest_cpuid_has_pcid(struct kvm_vcpu *vcpu)
{
	struct kvm_cpuid_entry2 *best;

	best = kvm_find_cpuid_entry(vcpu, 1, 0);
	return best && (best->ecx & bit(X86_FEATURE_PCID));
}

static inline bool guest_cpuid_has_x2apic(struct kvm_vcpu *vcpu)
{
	struct kvm_cpuid_entry2 *best;

	best = kvm_find_cpuid_entry(vcpu, 1, 0);
	return best && (best->ecx & bit(X86_FEATURE_X2APIC));
}

#endif