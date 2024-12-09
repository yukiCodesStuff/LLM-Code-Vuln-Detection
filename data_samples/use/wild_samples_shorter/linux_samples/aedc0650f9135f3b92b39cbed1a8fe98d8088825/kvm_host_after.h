
struct kvm_resize_hpt;

/* Flag values for kvm_arch.secure_guest */
#define KVMPPC_SECURE_INIT_START 0x1 /* H_SVM_INIT_START has been called */
#define KVMPPC_SECURE_INIT_DONE  0x2 /* H_SVM_INIT_DONE completed */

struct kvm_arch {
	unsigned int lpid;
	unsigned int smt_mode;		/* # vcpus per virtual core */
	unsigned int emul_smt_mode;	/* emualted SMT mode, on P9 */
#endif
	struct kvmppc_ops *kvm_ops;
#ifdef CONFIG_KVM_BOOK3S_HV_POSSIBLE
	struct mutex uvmem_lock;
	struct list_head uvmem_pfns;
	struct mutex mmu_setup_lock;	/* nests inside vcpu mutexes */
	u64 l1_ptcr;
	int max_nested_lpid;
	struct kvm_nested_guest *nested_guests[KVM_MAX_NESTED_GUESTS];