#define KVM_BUS_LOCK_DETECTION_VALID_MODE	(KVM_BUS_LOCK_DETECTION_OFF | \
						 KVM_BUS_LOCK_DETECTION_EXIT)

/* x86-specific vcpu->requests bit members */
#define KVM_REQ_MIGRATE_TIMER		KVM_ARCH_REQ(0)
#define KVM_REQ_REPORT_TPR_ACCESS	KVM_ARCH_REQ(1)
#define KVM_REQ_TRIPLE_FAULT		KVM_ARCH_REQ(2)

	bool bus_lock_detection_enabled;
	bool enable_pmu;
	/*
	 * If exit_on_emulation_error is set, and the in-kernel instruction
	 * emulator fails to emulate an instruction, allow userspace
	 * the opportunity to look at it.
	u64 directed_yield_attempted;
	u64 directed_yield_successful;
	u64 guest_mode;
};

struct x86_instruction_info;
