#define KVM_EXIT_X86_BUS_LOCK     33
#define KVM_EXIT_XEN              34
#define KVM_EXIT_RISCV_SBI        35
#define KVM_EXIT_NOTIFY           36

/* For KVM_EXIT_INTERNAL_ERROR */
/* Emulate instruction failed. */
#define KVM_INTERNAL_ERROR_EMULATION	1
			unsigned long args[6];
			unsigned long ret[2];
		} riscv_sbi;
		/* KVM_EXIT_NOTIFY */
		struct {
#define KVM_NOTIFY_CONTEXT_INVALID	(1 << 0)
			__u32 flags;
		} notify;
		/* Fix the size of the union. */
		char padding[256];
	};

#define KVM_CAP_ARM_SYSTEM_SUSPEND 216
#define KVM_CAP_S390_PROTECTED_DUMP 217
#define KVM_CAP_X86_TRIPLE_FAULT_EVENT 218
#define KVM_CAP_X86_NOTIFY_VMEXIT 219

#ifdef KVM_CAP_IRQ_ROUTING

struct kvm_irq_routing_irqchip {
/* Available with KVM_CAP_S390_PROTECTED_DUMP */
#define KVM_S390_PV_CPU_COMMAND	_IOWR(KVMIO, 0xd0, struct kvm_pv_cmd)

/* Available with KVM_CAP_X86_NOTIFY_VMEXIT */
#define KVM_X86_NOTIFY_VMEXIT_ENABLED		(1ULL << 0)
#define KVM_X86_NOTIFY_VMEXIT_USER		(1ULL << 1)

#endif /* __LINUX_KVM_H */