	u64  default_tsc_scaling_ratio;
	/* bus lock detection supported? */
	bool has_bus_lock_exit;
	/* notify VM exit supported? */
	bool has_notify_vmexit;

	u64 supported_mce_cap;
	u64 supported_xcr0;
	u64 supported_xss;
	return kvm->arch.cstate_in_guest;
}

static inline bool kvm_notify_vmexit_enabled(struct kvm *kvm)
{
	return kvm->arch.notify_vmexit_flags & KVM_X86_NOTIFY_VMEXIT_ENABLED;
}

enum kvm_intr_type {
	/* Values are arbitrary, but must be non-zero. */
	KVM_HANDLING_IRQ = 1,
	KVM_HANDLING_NMI,