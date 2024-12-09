	{ "irq_injections", VCPU_STAT(irq_injections) },
	{ "nmi_injections", VCPU_STAT(nmi_injections) },
	{ "req_event", VCPU_STAT(req_event) },
	{ "l1d_flush", VCPU_STAT(l1d_flush) },
	{ "mmu_shadow_zapped", VM_STAT(mmu_shadow_zapped) },
	{ "mmu_pte_write", VM_STAT(mmu_pte_write) },
	{ "mmu_pte_updated", VM_STAT(mmu_pte_updated) },
	{ "mmu_pde_zapped", VM_STAT(mmu_pde_zapped) },

static unsigned int num_msr_based_features;

u64 kvm_get_arch_capabilities(void)
{
	u64 data;

	rdmsrl_safe(MSR_IA32_ARCH_CAPABILITIES, &data);

	/*
	 * If we're doing cache flushes (either "always" or "cond")
	 * we will do one whenever the guest does a vmlaunch/vmresume.
	 * If an outer hypervisor is doing the cache flush for us
	 * (VMENTER_L1D_FLUSH_NESTED_VM), we can safely pass that
	 * capability to the guest too, and if EPT is disabled we're not
	 * vulnerable.  Overall, only VMENTER_L1D_FLUSH_NEVER will
	 * require a nested hypervisor to do a flush of its own.
	 */
	if (l1tf_vmx_mitigation != VMENTER_L1D_FLUSH_NEVER)
		data |= ARCH_CAP_SKIP_VMENTRY_L1DFLUSH;

	return data;
}
EXPORT_SYMBOL_GPL(kvm_get_arch_capabilities);

static int kvm_get_msr_feature(struct kvm_msr_entry *msr)
{
	switch (msr->index) {
	case MSR_IA32_ARCH_CAPABILITIES:
		msr->data = kvm_get_arch_capabilities();
		break;
	case MSR_IA32_UCODE_REV:
		rdmsrl_safe(msr->index, &msr->data);
		break;
	default:
		if (kvm_x86_ops->get_msr_feature(msr))
int kvm_write_guest_virt_system(struct kvm_vcpu *vcpu, gva_t addr, void *val,
				unsigned int bytes, struct x86_exception *exception)
{
	/* kvm_write_guest_virt_system can pull in tons of pages. */
	vcpu->arch.l1tf_flush_l1d = true;

	return kvm_write_guest_virt_helper(addr, val, bytes, vcpu,
					   PFERR_WRITE_MASK, exception);
}
EXPORT_SYMBOL_GPL(kvm_write_guest_virt_system);
	bool writeback = true;
	bool write_fault_to_spt = vcpu->arch.write_fault_to_shadow_pgtable;

	vcpu->arch.l1tf_flush_l1d = true;

	/*
	 * Clear write_fault_to_shadow_pgtable here to ensure it is
	 * never reused.
	 */
	struct kvm *kvm = vcpu->kvm;

	vcpu->srcu_idx = srcu_read_lock(&kvm->srcu);
	vcpu->arch.l1tf_flush_l1d = true;

	for (;;) {
		if (kvm_vcpu_running(vcpu)) {
			r = vcpu_enter_guest(vcpu);

void kvm_arch_sched_in(struct kvm_vcpu *vcpu, int cpu)
{
	vcpu->arch.l1tf_flush_l1d = true;
	kvm_x86_ops->sched_in(vcpu, cpu);
}

int kvm_arch_init_vm(struct kvm *kvm, unsigned long type)