struct kvm_caps {
	/* control of guest tsc rate supported? */
	bool has_tsc_control;
	/* maximum supported tsc_khz for guests */
	u32  max_guest_tsc_khz;
	/* number of bits of the fractional part of the TSC scaling ratio */
	u8   tsc_scaling_ratio_frac_bits;
	/* maximum allowed value of TSC scaling ratio */
	u64  max_tsc_scaling_ratio;
	/* 1ull << kvm_caps.tsc_scaling_ratio_frac_bits */
	u64  default_tsc_scaling_ratio;
	/* bus lock detection supported? */
	bool has_bus_lock_exit;

	u64 supported_mce_cap;
	u64 supported_xcr0;
	u64 supported_xss;
};

void kvm_spurious_fault(void);

#define KVM_NESTED_VMENTER_CONSISTENCY_CHECK(consistency_check)		\
({									\
	bool failed = (consistency_check);				\
	if (failed)							\
		trace_kvm_nested_vmenter_failed(#consistency_check, 0);	\
	failed;								\
})

#define KVM_DEFAULT_PLE_GAP		128
#define KVM_VMX_DEFAULT_PLE_WINDOW	4096
#define KVM_DEFAULT_PLE_WINDOW_GROW	2
#define KVM_DEFAULT_PLE_WINDOW_SHRINK	0
#define KVM_VMX_DEFAULT_PLE_WINDOW_MAX	UINT_MAX
#define KVM_SVM_DEFAULT_PLE_WINDOW_MAX	USHRT_MAX
#define KVM_SVM_DEFAULT_PLE_WINDOW	3000

static inline unsigned int __grow_ple_window(unsigned int val,
		unsigned int base, unsigned int modifier, unsigned int max)
{
	u64 ret = val;

	if (modifier < 1)
		return base;

	if (modifier < base)
		ret *= modifier;
	else
		ret += modifier;

	return min(ret, (u64)max);
}
{
	return kvm->arch.cstate_in_guest;
}

enum kvm_intr_type {
	/* Values are arbitrary, but must be non-zero. */
	KVM_HANDLING_IRQ = 1,
	KVM_HANDLING_NMI,
};

static inline void kvm_before_interrupt(struct kvm_vcpu *vcpu,
					enum kvm_intr_type intr)
{
	WRITE_ONCE(vcpu->arch.handling_intr_from_guest, (u8)intr);
}