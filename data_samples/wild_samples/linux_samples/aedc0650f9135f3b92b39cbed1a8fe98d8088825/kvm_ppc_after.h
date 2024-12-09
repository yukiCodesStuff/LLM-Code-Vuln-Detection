struct kvmppc_ops {
	struct module *owner;
	int (*get_sregs)(struct kvm_vcpu *vcpu, struct kvm_sregs *sregs);
	int (*set_sregs)(struct kvm_vcpu *vcpu, struct kvm_sregs *sregs);
	int (*get_one_reg)(struct kvm_vcpu *vcpu, u64 id,
			   union kvmppc_one_reg *val);
	int (*set_one_reg)(struct kvm_vcpu *vcpu, u64 id,
			   union kvmppc_one_reg *val);
	void (*vcpu_load)(struct kvm_vcpu *vcpu, int cpu);
	void (*vcpu_put)(struct kvm_vcpu *vcpu);
	void (*inject_interrupt)(struct kvm_vcpu *vcpu, int vec, u64 srr1_flags);
	void (*set_msr)(struct kvm_vcpu *vcpu, u64 msr);
	int (*vcpu_run)(struct kvm_run *run, struct kvm_vcpu *vcpu);
	struct kvm_vcpu *(*vcpu_create)(struct kvm *kvm, unsigned int id);
	void (*vcpu_free)(struct kvm_vcpu *vcpu);
	int (*check_requests)(struct kvm_vcpu *vcpu);
	int (*get_dirty_log)(struct kvm *kvm, struct kvm_dirty_log *log);
	void (*flush_memslot)(struct kvm *kvm, struct kvm_memory_slot *memslot);
	int (*prepare_memory_region)(struct kvm *kvm,
				     struct kvm_memory_slot *memslot,
				     const struct kvm_userspace_memory_region *mem);
	void (*commit_memory_region)(struct kvm *kvm,
				     const struct kvm_userspace_memory_region *mem,
				     const struct kvm_memory_slot *old,
				     const struct kvm_memory_slot *new,
				     enum kvm_mr_change change);
	int (*unmap_hva_range)(struct kvm *kvm, unsigned long start,
			   unsigned long end);
	int (*age_hva)(struct kvm *kvm, unsigned long start, unsigned long end);
	int (*test_age_hva)(struct kvm *kvm, unsigned long hva);
	void (*set_spte_hva)(struct kvm *kvm, unsigned long hva, pte_t pte);
	void (*mmu_destroy)(struct kvm_vcpu *vcpu);
	void (*free_memslot)(struct kvm_memory_slot *free,
			     struct kvm_memory_slot *dont);
	int (*create_memslot)(struct kvm_memory_slot *slot,
			      unsigned long npages);
	int (*init_vm)(struct kvm *kvm);
	void (*destroy_vm)(struct kvm *kvm);
	int (*get_smmu_info)(struct kvm *kvm, struct kvm_ppc_smmu_info *info);
	int (*emulate_op)(struct kvm_run *run, struct kvm_vcpu *vcpu,
			  unsigned int inst, int *advance);
	int (*emulate_mtspr)(struct kvm_vcpu *vcpu, int sprn, ulong spr_val);
	int (*emulate_mfspr)(struct kvm_vcpu *vcpu, int sprn, ulong *spr_val);
	void (*fast_vcpu_kick)(struct kvm_vcpu *vcpu);
	long (*arch_vm_ioctl)(struct file *filp, unsigned int ioctl,
			      unsigned long arg);
	int (*hcall_implemented)(unsigned long hcall);
	int (*irq_bypass_add_producer)(struct irq_bypass_consumer *,
				       struct irq_bypass_producer *);
	void (*irq_bypass_del_producer)(struct irq_bypass_consumer *,
					struct irq_bypass_producer *);
	int (*configure_mmu)(struct kvm *kvm, struct kvm_ppc_mmuv3_cfg *cfg);
	int (*get_rmmu_info)(struct kvm *kvm, struct kvm_ppc_rmmu_info *info);
	int (*set_smt_mode)(struct kvm *kvm, unsigned long mode,
			    unsigned long flags);
	void (*giveup_ext)(struct kvm_vcpu *vcpu, ulong msr);
	int (*enable_nested)(struct kvm *kvm);
	int (*load_from_eaddr)(struct kvm_vcpu *vcpu, ulong *eaddr, void *ptr,
			       int size);
	int (*store_to_eaddr)(struct kvm_vcpu *vcpu, ulong *eaddr, void *ptr,
			      int size);
	int (*svm_off)(struct kvm *kvm);
};

extern struct kvmppc_ops *kvmppc_hv_ops;
extern struct kvmppc_ops *kvmppc_pr_ops;

static inline int kvmppc_get_last_inst(struct kvm_vcpu *vcpu,
				enum instruction_fetch_type type, u32 *inst)
{
	int ret = EMULATE_DONE;
	u32 fetched_inst;

	/* Load the instruction manually if it failed to do so in the
	 * exit path */
	if (vcpu->arch.last_inst == KVM_INST_FETCH_FAILED)
		ret = kvmppc_load_last_inst(vcpu, type, &vcpu->arch.last_inst);

	/*  Write fetch_failed unswapped if the fetch failed */
	if (ret == EMULATE_DONE)
		fetched_inst = kvmppc_need_byteswap(vcpu) ?
				swab32(vcpu->arch.last_inst) :
				vcpu->arch.last_inst;
	else
		fetched_inst = vcpu->arch.last_inst;

	*inst = fetched_inst;
	return ret;
}