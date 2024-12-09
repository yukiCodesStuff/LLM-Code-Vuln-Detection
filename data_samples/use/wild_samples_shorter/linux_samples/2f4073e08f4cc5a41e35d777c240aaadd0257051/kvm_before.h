#define KVM_EXIT_X86_BUS_LOCK     33
#define KVM_EXIT_XEN              34
#define KVM_EXIT_RISCV_SBI        35

/* For KVM_EXIT_INTERNAL_ERROR */
/* Emulate instruction failed. */
#define KVM_INTERNAL_ERROR_EMULATION	1
			unsigned long args[6];
			unsigned long ret[2];
		} riscv_sbi;
		/* Fix the size of the union. */
		char padding[256];
	};

#define KVM_CAP_ARM_SYSTEM_SUSPEND 216
#define KVM_CAP_S390_PROTECTED_DUMP 217
#define KVM_CAP_X86_TRIPLE_FAULT_EVENT 218

#ifdef KVM_CAP_IRQ_ROUTING

struct kvm_irq_routing_irqchip {
/* Available with KVM_CAP_S390_PROTECTED_DUMP */
#define KVM_S390_PV_CPU_COMMAND	_IOWR(KVMIO, 0xd0, struct kvm_pv_cmd)

#endif /* __LINUX_KVM_H */