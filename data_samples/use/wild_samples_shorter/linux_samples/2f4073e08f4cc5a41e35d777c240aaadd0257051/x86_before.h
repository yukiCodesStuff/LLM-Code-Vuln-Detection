	u64  default_tsc_scaling_ratio;
	/* bus lock detection supported? */
	bool has_bus_lock_exit;

	u64 supported_mce_cap;
	u64 supported_xcr0;
	u64 supported_xss;
	return kvm->arch.cstate_in_guest;
}

enum kvm_intr_type {
	/* Values are arbitrary, but must be non-zero. */
	KVM_HANDLING_IRQ = 1,
	KVM_HANDLING_NMI,