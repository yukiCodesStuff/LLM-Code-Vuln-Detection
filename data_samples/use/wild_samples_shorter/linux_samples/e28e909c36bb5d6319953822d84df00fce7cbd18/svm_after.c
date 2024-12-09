#define TSC_RATIO_MIN		0x0000000000000001ULL
#define TSC_RATIO_MAX		0x000000ffffffffffULL

#define AVIC_HPA_MASK	~((0xFFFULL << 52) | 0xFFF)

/*
 * 0xff is broadcast, so the max index allowed for physical APIC ID
 * table is 0xfe.  APIC IDs above 0xff are reserved.
	u32 icrh = svm->vmcb->control.exit_info_1 >> 32;
	u32 icrl = svm->vmcb->control.exit_info_1;
	u32 id = svm->vmcb->control.exit_info_2 >> 32;
	u32 index = svm->vmcb->control.exit_info_2 & 0xFF;
	struct kvm_lapic *apic = svm->vcpu.arch.apic;

	trace_kvm_avic_incomplete_ipi(svm->vcpu.vcpu_id, icrh, icrl, id, index);
