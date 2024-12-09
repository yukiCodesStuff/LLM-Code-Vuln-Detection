#define PT_DIRECTORY_LEVEL 2
#define PT_PAGE_TABLE_LEVEL 1

#define PFERR_PRESENT_MASK (1U << 0)
#define PFERR_WRITE_MASK (1U << 1)
#define PFERR_USER_MASK (1U << 2)
#define PFERR_RSVD_MASK (1U << 3)
#define PFERR_FETCH_MASK (1U << 4)

int kvm_mmu_get_spte_hierarchy(struct kvm_vcpu *vcpu, u64 addr, u64 sptes[4]);
void kvm_mmu_set_mmio_spte_mask(u64 mmio_mask);

void kvm_init_shadow_mmu(struct kvm_vcpu *vcpu, struct kvm_mmu *context);
void kvm_init_shadow_ept_mmu(struct kvm_vcpu *vcpu, struct kvm_mmu *context,
		bool execonly);

static inline unsigned int kvm_mmu_available_pages(struct kvm *kvm)
{
	if (kvm->arch.n_max_mmu_pages > kvm->arch.n_used_mmu_pages)
 * Will a fault with a given page-fault error code (pfec) cause a permission
 * fault with the given access (in ACC_* format)?
 */
static inline bool permission_fault(struct kvm_mmu *mmu, unsigned pte_access,
				    unsigned pfec)
{
	return (mmu->permissions[pfec >> 1] >> pte_access) & 1;
}

void kvm_mmu_invalidate_zap_all_pages(struct kvm *kvm);
#endif