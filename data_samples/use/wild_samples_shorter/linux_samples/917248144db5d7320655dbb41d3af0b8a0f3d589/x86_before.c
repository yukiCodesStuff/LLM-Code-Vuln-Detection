void kvm_arch_vcpu_free(struct kvm_vcpu *vcpu)
{
	void *wbinvd_dirty_mask = vcpu->arch.wbinvd_dirty_mask;

	kvmclock_reset(vcpu);

	kvm_x86_ops->vcpu_free(vcpu);

void kvm_arch_memslots_updated(struct kvm *kvm, u64 gen)
{
	/*
	 * memslots->generation has been incremented.
	 * mmio generation may have reached its maximum value.
	 */
	kvm_mmu_invalidate_mmio_sptes(kvm, gen);
}

int kvm_arch_prepare_memory_region(struct kvm *kvm,
				struct kvm_memory_slot *memslot,