#define KVM_BUS_LOCK_DETECTION_VALID_MODE	(KVM_BUS_LOCK_DETECTION_OFF | \
						 KVM_BUS_LOCK_DETECTION_EXIT)

#define KVM_X86_NOTIFY_VMEXIT_VALID_BITS	(KVM_X86_NOTIFY_VMEXIT_ENABLED | \
						 KVM_X86_NOTIFY_VMEXIT_USER)

/* x86-specific vcpu->requests bit members */
#define KVM_REQ_MIGRATE_TIMER		KVM_ARCH_REQ(0)
#define KVM_REQ_REPORT_TPR_ACCESS	KVM_ARCH_REQ(1)
#define KVM_REQ_TRIPLE_FAULT		KVM_ARCH_REQ(2)

	bool bus_lock_detection_enabled;
	bool enable_pmu;

	u32 notify_window;
	u32 notify_vmexit_flags;
	/*
	 * If exit_on_emulation_error is set, and the in-kernel instruction
	 * emulator fails to emulate an instruction, allow userspace
	 * the opportunity to look at it.
	u64 directed_yield_attempted;
	u64 directed_yield_successful;
	u64 guest_mode;
	u64 notify_window_exits;
};

struct x86_instruction_info;
