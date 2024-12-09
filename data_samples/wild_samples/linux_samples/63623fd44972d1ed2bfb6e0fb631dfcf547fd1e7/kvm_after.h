struct kvm_sync_regs {
	/* Members of this structure are potentially malicious.
	 * Care must be taken by code reading, esp. interpreting,
	 * data fields from them inside KVM to prevent TOCTOU and
	 * double-fetch types of vulnerabilities.
	 */
	struct kvm_regs regs;
	struct kvm_sregs sregs;
	struct kvm_vcpu_events events;
};

#define KVM_X86_QUIRK_LINT0_REENABLED	   (1 << 0)
#define KVM_X86_QUIRK_CD_NW_CLEARED	   (1 << 1)
#define KVM_X86_QUIRK_LAPIC_MMIO_HOLE	   (1 << 2)
#define KVM_X86_QUIRK_OUT_7E_INC_RIP	   (1 << 3)
#define KVM_X86_QUIRK_MISC_ENABLE_NO_MWAIT (1 << 4)

#define KVM_STATE_NESTED_FORMAT_VMX	0
#define KVM_STATE_NESTED_FORMAT_SVM	1	/* unused */

#define KVM_STATE_NESTED_GUEST_MODE	0x00000001
#define KVM_STATE_NESTED_RUN_PENDING	0x00000002
#define KVM_STATE_NESTED_EVMCS		0x00000004
#define KVM_STATE_NESTED_MTF_PENDING	0x00000008

#define KVM_STATE_NESTED_SMM_GUEST_MODE	0x00000001
#define KVM_STATE_NESTED_SMM_VMXON	0x00000002

#define KVM_STATE_NESTED_VMX_VMCS_SIZE	0x1000

struct kvm_vmx_nested_state_data {
	__u8 vmcs12[KVM_STATE_NESTED_VMX_VMCS_SIZE];
	__u8 shadow_vmcs12[KVM_STATE_NESTED_VMX_VMCS_SIZE];
};

struct kvm_vmx_nested_state_hdr {
	__u64 vmxon_pa;
	__u64 vmcs12_pa;

	struct {
		__u16 flags;
	} smm;
};

/* for KVM_CAP_NESTED_STATE */
struct kvm_nested_state {
	__u16 flags;
	__u16 format;
	__u32 size;

	union {
		struct kvm_vmx_nested_state_hdr vmx;

		/* Pad the header to 128 bytes.  */
		__u8 pad[120];
	} hdr;

	/*
	 * Define data region as 0 bytes to preserve backwards-compatability
	 * to old definition of kvm_nested_state in order to avoid changing
	 * KVM_{GET,PUT}_NESTED_STATE ioctl values.
	 */
	union {
		struct kvm_vmx_nested_state_data vmx[0];
	} data;
};

/* for KVM_CAP_PMU_EVENT_FILTER */
struct kvm_pmu_event_filter {
	__u32 action;
	__u32 nevents;
	__u32 fixed_counter_bitmap;
	__u32 flags;
	__u32 pad[4];
	__u64 events[0];
};

#define KVM_PMU_EVENT_ALLOW 0
#define KVM_PMU_EVENT_DENY 1

#endif /* _ASM_X86_KVM_H */