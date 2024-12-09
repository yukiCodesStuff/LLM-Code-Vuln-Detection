{
	kvm_queue_exception_e(vcpu, GP_VECTOR, error_code);
}

static inline u64 get_canonical(u64 la)
{
	return ((int64_t)la << 16) >> 16;
}

static inline bool is_noncanonical_address(u64 la)
{
#ifdef CONFIG_X86_64
	return get_canonical(la) != la;
#else
	return false;
#endif
}

#define TSS_IOPB_BASE_OFFSET 0x66
#define TSS_BASE_SIZE 0x68
#define TSS_IOPB_SIZE (65536 / 8)
#define TSS_REDIRECTION_SIZE (256 / 8)
#define RMODE_TSS_SIZE							\
	(TSS_BASE_SIZE + TSS_REDIRECTION_SIZE + TSS_IOPB_SIZE + 1)

enum {
	TASK_SWITCH_CALL = 0,
	TASK_SWITCH_IRET = 1,
	TASK_SWITCH_JMP = 2,
	TASK_SWITCH_GATE = 3,
};

#define HF_GIF_MASK		(1 << 0)
#define HF_HIF_MASK		(1 << 1)
#define HF_VINTR_MASK		(1 << 2)
#define HF_NMI_MASK		(1 << 3)
#define HF_IRET_MASK		(1 << 4)
#define HF_GUEST_MASK		(1 << 5) /* VCPU is in guest-mode */

/*
 * Hardware virtualization extension instructions may fault if a
 * reboot turns off virtualization while processes are running.
 * Trap the fault and ignore the instruction if that happens.
 */
asmlinkage void kvm_spurious_fault(void);

#define ____kvm_handle_fault_on_reboot(insn, cleanup_insn)	\
	"666: " insn "\n\t" \
	"668: \n\t"                           \
	".pushsection .fixup, \"ax\" \n" \
	"667: \n\t" \
	cleanup_insn "\n\t"		      \
	"cmpb $0, kvm_rebooting \n\t"	      \
	"jne 668b \n\t"      		      \
	__ASM_SIZE(push) " $666b \n\t"	      \
	"call kvm_spurious_fault \n\t"	      \
	".popsection \n\t" \
	_ASM_EXTABLE(666b, 667b)

#define __kvm_handle_fault_on_reboot(insn)		\
	____kvm_handle_fault_on_reboot(insn, "")

#define KVM_ARCH_WANT_MMU_NOTIFIER
int kvm_unmap_hva(struct kvm *kvm, unsigned long hva);
int kvm_unmap_hva_range(struct kvm *kvm, unsigned long start, unsigned long end);
int kvm_age_hva(struct kvm *kvm, unsigned long start, unsigned long end);
int kvm_test_age_hva(struct kvm *kvm, unsigned long hva);
void kvm_set_spte_hva(struct kvm *kvm, unsigned long hva, pte_t pte);
int cpuid_maxphyaddr(struct kvm_vcpu *vcpu);
int kvm_cpu_has_injectable_intr(struct kvm_vcpu *v);
int kvm_cpu_has_interrupt(struct kvm_vcpu *vcpu);
int kvm_arch_interrupt_allowed(struct kvm_vcpu *vcpu);
int kvm_cpu_get_interrupt(struct kvm_vcpu *v);
void kvm_vcpu_reset(struct kvm_vcpu *vcpu);
void kvm_vcpu_reload_apic_access_page(struct kvm_vcpu *vcpu);
void kvm_arch_mmu_notifier_invalidate_page(struct kvm *kvm,
					   unsigned long address);

void kvm_define_shared_msr(unsigned index, u32 msr);
void kvm_set_shared_msr(unsigned index, u64 val, u64 mask);

bool kvm_is_linear_rip(struct kvm_vcpu *vcpu, unsigned long linear_rip);

void kvm_arch_async_page_not_present(struct kvm_vcpu *vcpu,
				     struct kvm_async_pf *work);
void kvm_arch_async_page_present(struct kvm_vcpu *vcpu,
				 struct kvm_async_pf *work);
void kvm_arch_async_page_ready(struct kvm_vcpu *vcpu,
			       struct kvm_async_pf *work);
bool kvm_arch_can_inject_async_page_present(struct kvm_vcpu *vcpu);
extern bool kvm_find_async_pf_gfn(struct kvm_vcpu *vcpu, gfn_t gfn);

void kvm_complete_insn_gp(struct kvm_vcpu *vcpu, int err);

int kvm_is_in_guest(void);

void kvm_pmu_init(struct kvm_vcpu *vcpu);
void kvm_pmu_destroy(struct kvm_vcpu *vcpu);
void kvm_pmu_reset(struct kvm_vcpu *vcpu);
void kvm_pmu_cpuid_update(struct kvm_vcpu *vcpu);
bool kvm_pmu_msr(struct kvm_vcpu *vcpu, u32 msr);
int kvm_pmu_get_msr(struct kvm_vcpu *vcpu, u32 msr, u64 *data);
int kvm_pmu_set_msr(struct kvm_vcpu *vcpu, struct msr_data *msr_info);
int kvm_pmu_check_pmc(struct kvm_vcpu *vcpu, unsigned pmc);
int kvm_pmu_read_pmc(struct kvm_vcpu *vcpu, unsigned pmc, u64 *data);
void kvm_handle_pmu_event(struct kvm_vcpu *vcpu);
void kvm_deliver_pmi(struct kvm_vcpu *vcpu);

#endif /* _ASM_X86_KVM_HOST_H */