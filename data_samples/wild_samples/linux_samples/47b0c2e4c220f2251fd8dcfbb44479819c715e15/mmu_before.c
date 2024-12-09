{
	bool list_unstable, zapped_root = false;

	trace_kvm_mmu_prepare_zap_page(sp);
	++kvm->stat.mmu_shadow_zapped;
	*nr_zapped = mmu_zap_unsync_children(kvm, sp, invalid_list);
	*nr_zapped += kvm_mmu_page_unlink_children(kvm, sp, invalid_list);
	kvm_mmu_unlink_parents(sp);

	/* Zapping children means active_mmu_pages has become unstable. */
	list_unstable = *nr_zapped;

	if (!sp->role.invalid && sp_has_gptes(sp))
		unaccount_shadowed(kvm, sp);

	if (sp->unsync)
		kvm_unlink_unsync_page(kvm, sp);
	if (!sp->root_count) {
		/* Count self */
		(*nr_zapped)++;

		/*
		 * Already invalid pages (previously active roots) are not on
		 * the active page list.  See list_del() in the "else" case of
		 * !sp->root_count.
		 */
		if (sp->role.invalid)
			list_add(&sp->link, invalid_list);
		else
			list_move(&sp->link, invalid_list);
		kvm_unaccount_mmu_page(kvm, sp);
	} else {
		/*
		 * Remove the active root from the active page list, the root
		 * will be explicitly freed when the root_count hits zero.
		 */
		list_del(&sp->link);

		/*
		 * Obsolete pages cannot be used on any vCPUs, see the comment
		 * in kvm_mmu_zap_all_fast().  Note, is_obsolete_sp() also
		 * treats invalid shadow pages as being obsolete.
		 */
		zapped_root = !is_obsolete_sp(kvm, sp);
	}

	if (sp->lpage_disallowed)
		unaccount_huge_nx_page(kvm, sp);

	sp->role.invalid = 1;

	/*
	 * Make the request to free obsolete roots after marking the root
	 * invalid, otherwise other vCPUs may not see it as invalid.
	 */
	if (zapped_root)
		kvm_make_all_cpus_request(kvm, KVM_REQ_MMU_FREE_OBSOLETE_ROOTS);
	return list_unstable;
}
{
	bool is_tdp_mmu_fault = is_tdp_mmu(vcpu->arch.mmu);

	unsigned long mmu_seq;
	int r;

	fault->gfn = fault->addr >> PAGE_SHIFT;
	fault->slot = kvm_vcpu_gfn_to_memslot(vcpu, fault->gfn);

	if (page_fault_handle_page_track(vcpu, fault))
		return RET_PF_EMULATE;

	r = fast_page_fault(vcpu, fault);
	if (r != RET_PF_INVALID)
		return r;

	r = mmu_topup_memory_caches(vcpu, false);
	if (r)
		return r;

	mmu_seq = vcpu->kvm->mmu_invalidate_seq;
	smp_rmb();

	r = kvm_faultin_pfn(vcpu, fault);
	if (r != RET_PF_CONTINUE)
		return r;

	r = handle_abnormal_pfn(vcpu, fault, ACC_ALL);
	if (r != RET_PF_CONTINUE)
		return r;

	r = RET_PF_RETRY;

	if (is_tdp_mmu_fault)
		read_lock(&vcpu->kvm->mmu_lock);
	else
		write_lock(&vcpu->kvm->mmu_lock);

	if (is_page_fault_stale(vcpu, fault, mmu_seq))
		goto out_unlock;

	r = make_mmu_pages_available(vcpu);
	if (r)
		goto out_unlock;

	if (is_tdp_mmu_fault)
		r = kvm_tdp_mmu_map(vcpu, fault);
	else
		r = __direct_map(vcpu, fault);

out_unlock:
	if (is_tdp_mmu_fault)
		read_unlock(&vcpu->kvm->mmu_lock);
	else
		write_unlock(&vcpu->kvm->mmu_lock);
	kvm_release_pfn_clean(fault->pfn);
	return r;
}

static int nonpaging_page_fault(struct kvm_vcpu *vcpu,
				struct kvm_page_fault *fault)
{
	pgprintk("%s: gva %lx error %x\n", __func__, fault->addr, fault->error_code);

	/* This path builds a PAE pagetable, we can map 2mb pages at maximum. */
	fault->max_level = PG_LEVEL_2M;
	return direct_page_fault(vcpu, fault);
}

int kvm_handle_page_fault(struct kvm_vcpu *vcpu, u64 error_code,
				u64 fault_address, char *insn, int insn_len)
{
	int r = 1;
	u32 flags = vcpu->arch.apf.host_apf_flags;

#ifndef CONFIG_X86_64
	/* A 64-bit CR2 should be impossible on 32-bit KVM. */
	if (WARN_ON_ONCE(fault_address >> 32))
		return -EFAULT;
#endif

	vcpu->arch.l1tf_flush_l1d = true;
	if (!flags) {
		trace_kvm_page_fault(vcpu, fault_address, error_code);

		if (kvm_event_needs_reinjection(vcpu))
			kvm_mmu_unprotect_page_virt(vcpu, fault_address);
		r = kvm_mmu_page_fault(vcpu, fault_address, error_code, insn,
				insn_len);
	} else if (flags & KVM_PV_REASON_PAGE_NOT_PRESENT) {
		vcpu->arch.apf.host_apf_flags = 0;
		local_irq_disable();
		kvm_async_pf_task_wait_schedule(fault_address);
		local_irq_enable();
	} else {
		WARN_ONCE(1, "Unexpected host async PF flags: %x\n", flags);
	}

	return r;
}
EXPORT_SYMBOL_GPL(kvm_handle_page_fault);

int kvm_tdp_page_fault(struct kvm_vcpu *vcpu, struct kvm_page_fault *fault)
{
	/*
	 * If the guest's MTRRs may be used to compute the "real" memtype,
	 * restrict the mapping level to ensure KVM uses a consistent memtype
	 * across the entire mapping.  If the host MTRRs are ignored by TDP
	 * (shadow_memtype_mask is non-zero), and the VM has non-coherent DMA
	 * (DMA doesn't snoop CPU caches), KVM's ABI is to honor the memtype
	 * from the guest's MTRRs so that guest accesses to memory that is
	 * DMA'd aren't cached against the guest's wishes.
	 *
	 * Note, KVM may still ultimately ignore guest MTRRs for certain PFNs,
	 * e.g. KVM will force UC memtype for host MMIO.
	 */
	if (shadow_memtype_mask && kvm_arch_has_noncoherent_dma(vcpu->kvm)) {
		for ( ; fault->max_level > PG_LEVEL_4K; --fault->max_level) {
			int page_num = KVM_PAGES_PER_HPAGE(fault->max_level);
			gfn_t base = (fault->addr >> PAGE_SHIFT) & ~(page_num - 1);

			if (kvm_mtrr_check_gfn_range_consistency(vcpu, base, page_num))
				break;
		}
	}

	return direct_page_fault(vcpu, fault);
}

static void nonpaging_init_context(struct kvm_mmu *context)
{
	context->page_fault = nonpaging_page_fault;
	context->gva_to_gpa = nonpaging_gva_to_gpa;
	context->sync_page = nonpaging_sync_page;
	context->invlpg = NULL;
}

static inline bool is_root_usable(struct kvm_mmu_root_info *root, gpa_t pgd,
				  union kvm_mmu_page_role role)
{
	return (role.direct || pgd == root->pgd) &&
	       VALID_PAGE(root->hpa) &&
	       role.word == to_shadow_page(root->hpa)->role.word;
}

/*
 * Find out if a previously cached root matching the new pgd/role is available,
 * and insert the current root as the MRU in the cache.
 * If a matching root is found, it is assigned to kvm_mmu->root and
 * true is returned.
 * If no match is found, kvm_mmu->root is left invalid, the LRU root is
 * evicted to make room for the current root, and false is returned.
 */
static bool cached_root_find_and_keep_current(struct kvm *kvm, struct kvm_mmu *mmu,
					      gpa_t new_pgd,
					      union kvm_mmu_page_role new_role)
{
	uint i;

	if (is_root_usable(&mmu->root, new_pgd, new_role))
		return true;

	for (i = 0; i < KVM_MMU_NUM_PREV_ROOTS; i++) {
		/*
		 * The swaps end up rotating the cache like this:
		 *   C   0 1 2 3   (on entry to the function)
		 *   0   C 1 2 3
		 *   1   C 0 2 3
		 *   2   C 0 1 3
		 *   3   C 0 1 2   (on exit from the loop)
		 */
		swap(mmu->root, mmu->prev_roots[i]);
		if (is_root_usable(&mmu->root, new_pgd, new_role))
			return true;
	}

	kvm_mmu_free_roots(kvm, mmu, KVM_MMU_ROOT_CURRENT);
	return false;
}

/*
 * Find out if a previously cached root matching the new pgd/role is available.
 * On entry, mmu->root is invalid.
 * If a matching root is found, it is assigned to kvm_mmu->root, the LRU entry
 * of the cache becomes invalid, and true is returned.
 * If no match is found, kvm_mmu->root is left invalid and false is returned.
 */
static bool cached_root_find_without_current(struct kvm *kvm, struct kvm_mmu *mmu,
					     gpa_t new_pgd,
					     union kvm_mmu_page_role new_role)
{
	uint i;

	for (i = 0; i < KVM_MMU_NUM_PREV_ROOTS; i++)
		if (is_root_usable(&mmu->prev_roots[i], new_pgd, new_role))
			goto hit;

	return false;

hit:
	swap(mmu->root, mmu->prev_roots[i]);
	/* Bubble up the remaining roots.  */
	for (; i < KVM_MMU_NUM_PREV_ROOTS - 1; i++)
		mmu->prev_roots[i] = mmu->prev_roots[i + 1];
	mmu->prev_roots[i].hpa = INVALID_PAGE;
	return true;
}

static bool fast_pgd_switch(struct kvm *kvm, struct kvm_mmu *mmu,
			    gpa_t new_pgd, union kvm_mmu_page_role new_role)
{
	/*
	 * For now, limit the caching to 64-bit hosts+VMs in order to avoid
	 * having to deal with PDPTEs. We may add support for 32-bit hosts/VMs
	 * later if necessary.
	 */
	if (VALID_PAGE(mmu->root.hpa) && !to_shadow_page(mmu->root.hpa))
		kvm_mmu_free_roots(kvm, mmu, KVM_MMU_ROOT_CURRENT);

	if (VALID_PAGE(mmu->root.hpa))
		return cached_root_find_and_keep_current(kvm, mmu, new_pgd, new_role);
	else
		return cached_root_find_without_current(kvm, mmu, new_pgd, new_role);
}

void kvm_mmu_new_pgd(struct kvm_vcpu *vcpu, gpa_t new_pgd)
{
	struct kvm_mmu *mmu = vcpu->arch.mmu;
	union kvm_mmu_page_role new_role = mmu->root_role;

	if (!fast_pgd_switch(vcpu->kvm, mmu, new_pgd, new_role)) {
		/* kvm_mmu_ensure_valid_pgd will set up a new root.  */
		return;
	}

	/*
	 * It's possible that the cached previous root page is obsolete because
	 * of a change in the MMU generation number. However, changing the
	 * generation number is accompanied by KVM_REQ_MMU_FREE_OBSOLETE_ROOTS,
	 * which will free the root set here and allocate a new one.
	 */
	kvm_make_request(KVM_REQ_LOAD_MMU_PGD, vcpu);

	if (force_flush_and_sync_on_reuse) {
		kvm_make_request(KVM_REQ_MMU_SYNC, vcpu);
		kvm_make_request(KVM_REQ_TLB_FLUSH_CURRENT, vcpu);
	}

	/*
	 * The last MMIO access's GVA and GPA are cached in the VCPU. When
	 * switching to a new CR3, that GVA->GPA mapping may no longer be
	 * valid. So clear any cached MMIO info even when we don't need to sync
	 * the shadow page tables.
	 */
	vcpu_clear_mmio_info(vcpu, MMIO_GVA_ANY);

	/*
	 * If this is a direct root page, it doesn't have a write flooding
	 * count. Otherwise, clear the write flooding count.
	 */
	if (!new_role.direct)
		__clear_sp_write_flooding_count(
				to_shadow_page(vcpu->arch.mmu->root.hpa));
}
EXPORT_SYMBOL_GPL(kvm_mmu_new_pgd);

static unsigned long get_cr3(struct kvm_vcpu *vcpu)
{
	return kvm_read_cr3(vcpu);
}

static bool sync_mmio_spte(struct kvm_vcpu *vcpu, u64 *sptep, gfn_t gfn,
			   unsigned int access)
{
	if (unlikely(is_mmio_spte(*sptep))) {
		if (gfn != get_mmio_spte_gfn(*sptep)) {
			mmu_spte_clear_no_track(sptep);
			return true;
		}

		mark_mmio_spte(vcpu, sptep, gfn, access);
		return true;
	}

	return false;
}

#define PTTYPE_EPT 18 /* arbitrary */
#define PTTYPE PTTYPE_EPT
#include "paging_tmpl.h"
#undef PTTYPE

#define PTTYPE 64
#include "paging_tmpl.h"
#undef PTTYPE

#define PTTYPE 32
#include "paging_tmpl.h"
#undef PTTYPE

static void
__reset_rsvds_bits_mask(struct rsvd_bits_validate *rsvd_check,
			u64 pa_bits_rsvd, int level, bool nx, bool gbpages,
			bool pse, bool amd)
{
	u64 gbpages_bit_rsvd = 0;
	u64 nonleaf_bit8_rsvd = 0;
	u64 high_bits_rsvd;

	rsvd_check->bad_mt_xwr = 0;

	if (!gbpages)
		gbpages_bit_rsvd = rsvd_bits(7, 7);

	if (level == PT32E_ROOT_LEVEL)
		high_bits_rsvd = pa_bits_rsvd & rsvd_bits(0, 62);
	else
		high_bits_rsvd = pa_bits_rsvd & rsvd_bits(0, 51);

	/* Note, NX doesn't exist in PDPTEs, this is handled below. */
	if (!nx)
		high_bits_rsvd |= rsvd_bits(63, 63);

	/*
	 * Non-leaf PML4Es and PDPEs reserve bit 8 (which would be the G bit for
	 * leaf entries) on AMD CPUs only.
	 */
	if (amd)
		nonleaf_bit8_rsvd = rsvd_bits(8, 8);

	switch (level) {
	case PT32_ROOT_LEVEL:
		/* no rsvd bits for 2 level 4K page table entries */
		rsvd_check->rsvd_bits_mask[0][1] = 0;
		rsvd_check->rsvd_bits_mask[0][0] = 0;
		rsvd_check->rsvd_bits_mask[1][0] =
			rsvd_check->rsvd_bits_mask[0][0];

		if (!pse) {
			rsvd_check->rsvd_bits_mask[1][1] = 0;
			break;
		}

		if (is_cpuid_PSE36())
			/* 36bits PSE 4MB page */
			rsvd_check->rsvd_bits_mask[1][1] = rsvd_bits(17, 21);
		else
			/* 32 bits PSE 4MB page */
			rsvd_check->rsvd_bits_mask[1][1] = rsvd_bits(13, 21);
		break;
	case PT32E_ROOT_LEVEL:
		rsvd_check->rsvd_bits_mask[0][2] = rsvd_bits(63, 63) |
						   high_bits_rsvd |
						   rsvd_bits(5, 8) |
						   rsvd_bits(1, 2);	/* PDPTE */
		rsvd_check->rsvd_bits_mask[0][1] = high_bits_rsvd;	/* PDE */
		rsvd_check->rsvd_bits_mask[0][0] = high_bits_rsvd;	/* PTE */
		rsvd_check->rsvd_bits_mask[1][1] = high_bits_rsvd |
						   rsvd_bits(13, 20);	/* large page */
		rsvd_check->rsvd_bits_mask[1][0] =
			rsvd_check->rsvd_bits_mask[0][0];
		break;
	case PT64_ROOT_5LEVEL:
		rsvd_check->rsvd_bits_mask[0][4] = high_bits_rsvd |
						   nonleaf_bit8_rsvd |
						   rsvd_bits(7, 7);
		rsvd_check->rsvd_bits_mask[1][4] =
			rsvd_check->rsvd_bits_mask[0][4];
		fallthrough;
	case PT64_ROOT_4LEVEL:
		rsvd_check->rsvd_bits_mask[0][3] = high_bits_rsvd |
						   nonleaf_bit8_rsvd |
						   rsvd_bits(7, 7);
		rsvd_check->rsvd_bits_mask[0][2] = high_bits_rsvd |
						   gbpages_bit_rsvd;
		rsvd_check->rsvd_bits_mask[0][1] = high_bits_rsvd;
		rsvd_check->rsvd_bits_mask[0][0] = high_bits_rsvd;
		rsvd_check->rsvd_bits_mask[1][3] =
			rsvd_check->rsvd_bits_mask[0][3];
		rsvd_check->rsvd_bits_mask[1][2] = high_bits_rsvd |
						   gbpages_bit_rsvd |
						   rsvd_bits(13, 29);
		rsvd_check->rsvd_bits_mask[1][1] = high_bits_rsvd |
						   rsvd_bits(13, 20); /* large page */
		rsvd_check->rsvd_bits_mask[1][0] =
			rsvd_check->rsvd_bits_mask[0][0];
		break;
	}
}

static bool guest_can_use_gbpages(struct kvm_vcpu *vcpu)
{
	/*
	 * If TDP is enabled, let the guest use GBPAGES if they're supported in
	 * hardware.  The hardware page walker doesn't let KVM disable GBPAGES,
	 * i.e. won't treat them as reserved, and KVM doesn't redo the GVA->GPA
	 * walk for performance and complexity reasons.  Not to mention KVM
	 * _can't_ solve the problem because GVA->GPA walks aren't visible to
	 * KVM once a TDP translation is installed.  Mimic hardware behavior so
	 * that KVM's is at least consistent, i.e. doesn't randomly inject #PF.
	 */
	return tdp_enabled ? boot_cpu_has(X86_FEATURE_GBPAGES) :
			     guest_cpuid_has(vcpu, X86_FEATURE_GBPAGES);
}

static void reset_guest_rsvds_bits_mask(struct kvm_vcpu *vcpu,
					struct kvm_mmu *context)
{
	__reset_rsvds_bits_mask(&context->guest_rsvd_check,
				vcpu->arch.reserved_gpa_bits,
				context->cpu_role.base.level, is_efer_nx(context),
				guest_can_use_gbpages(vcpu),
				is_cr4_pse(context),
				guest_cpuid_is_amd_or_hygon(vcpu));
}

static void
__reset_rsvds_bits_mask_ept(struct rsvd_bits_validate *rsvd_check,
			    u64 pa_bits_rsvd, bool execonly, int huge_page_level)
{
	u64 high_bits_rsvd = pa_bits_rsvd & rsvd_bits(0, 51);
	u64 large_1g_rsvd = 0, large_2m_rsvd = 0;
	u64 bad_mt_xwr;

	if (huge_page_level < PG_LEVEL_1G)
		large_1g_rsvd = rsvd_bits(7, 7);
	if (huge_page_level < PG_LEVEL_2M)
		large_2m_rsvd = rsvd_bits(7, 7);

	rsvd_check->rsvd_bits_mask[0][4] = high_bits_rsvd | rsvd_bits(3, 7);
	rsvd_check->rsvd_bits_mask[0][3] = high_bits_rsvd | rsvd_bits(3, 7);
	rsvd_check->rsvd_bits_mask[0][2] = high_bits_rsvd | rsvd_bits(3, 6) | large_1g_rsvd;
	rsvd_check->rsvd_bits_mask[0][1] = high_bits_rsvd | rsvd_bits(3, 6) | large_2m_rsvd;
	rsvd_check->rsvd_bits_mask[0][0] = high_bits_rsvd;

	/* large page */
	rsvd_check->rsvd_bits_mask[1][4] = rsvd_check->rsvd_bits_mask[0][4];
	rsvd_check->rsvd_bits_mask[1][3] = rsvd_check->rsvd_bits_mask[0][3];
	rsvd_check->rsvd_bits_mask[1][2] = high_bits_rsvd | rsvd_bits(12, 29) | large_1g_rsvd;
	rsvd_check->rsvd_bits_mask[1][1] = high_bits_rsvd | rsvd_bits(12, 20) | large_2m_rsvd;
	rsvd_check->rsvd_bits_mask[1][0] = rsvd_check->rsvd_bits_mask[0][0];

	bad_mt_xwr = 0xFFull << (2 * 8);	/* bits 3..5 must not be 2 */
	bad_mt_xwr |= 0xFFull << (3 * 8);	/* bits 3..5 must not be 3 */
	bad_mt_xwr |= 0xFFull << (7 * 8);	/* bits 3..5 must not be 7 */
	bad_mt_xwr |= REPEAT_BYTE(1ull << 2);	/* bits 0..2 must not be 010 */
	bad_mt_xwr |= REPEAT_BYTE(1ull << 6);	/* bits 0..2 must not be 110 */
	if (!execonly) {
		/* bits 0..2 must not be 100 unless VMX capabilities allow it */
		bad_mt_xwr |= REPEAT_BYTE(1ull << 4);
	}
	rsvd_check->bad_mt_xwr = bad_mt_xwr;
}

static void reset_rsvds_bits_mask_ept(struct kvm_vcpu *vcpu,
		struct kvm_mmu *context, bool execonly, int huge_page_level)
{
	__reset_rsvds_bits_mask_ept(&context->guest_rsvd_check,
				    vcpu->arch.reserved_gpa_bits, execonly,
				    huge_page_level);
}

static inline u64 reserved_hpa_bits(void)
{
	return rsvd_bits(shadow_phys_bits, 63);
}

/*
 * the page table on host is the shadow page table for the page
 * table in guest or amd nested guest, its mmu features completely
 * follow the features in guest.
 */
static void reset_shadow_zero_bits_mask(struct kvm_vcpu *vcpu,
					struct kvm_mmu *context)
{
	/* @amd adds a check on bit of SPTEs, which KVM shouldn't use anyways. */
	bool is_amd = true;
	/* KVM doesn't use 2-level page tables for the shadow MMU. */
	bool is_pse = false;
	struct rsvd_bits_validate *shadow_zero_check;
	int i;

	WARN_ON_ONCE(context->root_role.level < PT32E_ROOT_LEVEL);

	shadow_zero_check = &context->shadow_zero_check;
	__reset_rsvds_bits_mask(shadow_zero_check, reserved_hpa_bits(),
				context->root_role.level,
				context->root_role.efer_nx,
				guest_can_use_gbpages(vcpu), is_pse, is_amd);

	if (!shadow_me_mask)
		return;

	for (i = context->root_role.level; --i >= 0;) {
		/*
		 * So far shadow_me_value is a constant during KVM's life
		 * time.  Bits in shadow_me_value are allowed to be set.
		 * Bits in shadow_me_mask but not in shadow_me_value are
		 * not allowed to be set.
		 */
		shadow_zero_check->rsvd_bits_mask[0][i] |= shadow_me_mask;
		shadow_zero_check->rsvd_bits_mask[1][i] |= shadow_me_mask;
		shadow_zero_check->rsvd_bits_mask[0][i] &= ~shadow_me_value;
		shadow_zero_check->rsvd_bits_mask[1][i] &= ~shadow_me_value;
	}

}

static inline bool boot_cpu_is_amd(void)
{
	WARN_ON_ONCE(!tdp_enabled);
	return shadow_x_mask == 0;
}

/*
 * the direct page table on host, use as much mmu features as
 * possible, however, kvm currently does not do execution-protection.
 */
static void
reset_tdp_shadow_zero_bits_mask(struct kvm_mmu *context)
{
	struct rsvd_bits_validate *shadow_zero_check;
	int i;

	shadow_zero_check = &context->shadow_zero_check;

	if (boot_cpu_is_amd())
		__reset_rsvds_bits_mask(shadow_zero_check, reserved_hpa_bits(),
					context->root_role.level, true,
					boot_cpu_has(X86_FEATURE_GBPAGES),
					false, true);
	else
		__reset_rsvds_bits_mask_ept(shadow_zero_check,
					    reserved_hpa_bits(), false,
					    max_huge_page_level);

	if (!shadow_me_mask)
		return;

	for (i = context->root_role.level; --i >= 0;) {
		shadow_zero_check->rsvd_bits_mask[0][i] &= ~shadow_me_mask;
		shadow_zero_check->rsvd_bits_mask[1][i] &= ~shadow_me_mask;
	}
}

/*
 * as the comments in reset_shadow_zero_bits_mask() except it
 * is the shadow page table for intel nested guest.
 */
static void
reset_ept_shadow_zero_bits_mask(struct kvm_mmu *context, bool execonly)
{
	__reset_rsvds_bits_mask_ept(&context->shadow_zero_check,
				    reserved_hpa_bits(), execonly,
				    max_huge_page_level);
}

#define BYTE_MASK(access) \
	((1 & (access) ? 2 : 0) | \
	 (2 & (access) ? 4 : 0) | \
	 (3 & (access) ? 8 : 0) | \
	 (4 & (access) ? 16 : 0) | \
	 (5 & (access) ? 32 : 0) | \
	 (6 & (access) ? 64 : 0) | \
	 (7 & (access) ? 128 : 0))


static void update_permission_bitmask(struct kvm_mmu *mmu, bool ept)
{
	unsigned byte;

	const u8 x = BYTE_MASK(ACC_EXEC_MASK);
	const u8 w = BYTE_MASK(ACC_WRITE_MASK);
	const u8 u = BYTE_MASK(ACC_USER_MASK);

	bool cr4_smep = is_cr4_smep(mmu);
	bool cr4_smap = is_cr4_smap(mmu);
	bool cr0_wp = is_cr0_wp(mmu);
	bool efer_nx = is_efer_nx(mmu);

	for (byte = 0; byte < ARRAY_SIZE(mmu->permissions); ++byte) {
		unsigned pfec = byte << 1;

		/*
		 * Each "*f" variable has a 1 bit for each UWX value
		 * that causes a fault with the given PFEC.
		 */

		/* Faults from writes to non-writable pages */
		u8 wf = (pfec & PFERR_WRITE_MASK) ? (u8)~w : 0;
		/* Faults from user mode accesses to supervisor pages */
		u8 uf = (pfec & PFERR_USER_MASK) ? (u8)~u : 0;
		/* Faults from fetches of non-executable pages*/
		u8 ff = (pfec & PFERR_FETCH_MASK) ? (u8)~x : 0;
		/* Faults from kernel mode fetches of user pages */
		u8 smepf = 0;
		/* Faults from kernel mode accesses of user pages */
		u8 smapf = 0;

		if (!ept) {
			/* Faults from kernel mode accesses to user pages */
			u8 kf = (pfec & PFERR_USER_MASK) ? 0 : u;

			/* Not really needed: !nx will cause pte.nx to fault */
			if (!efer_nx)
				ff = 0;

			/* Allow supervisor writes if !cr0.wp */
			if (!cr0_wp)
				wf = (pfec & PFERR_USER_MASK) ? wf : 0;

			/* Disallow supervisor fetches of user code if cr4.smep */
			if (cr4_smep)
				smepf = (pfec & PFERR_FETCH_MASK) ? kf : 0;

			/*
			 * SMAP:kernel-mode data accesses from user-mode
			 * mappings should fault. A fault is considered
			 * as a SMAP violation if all of the following
			 * conditions are true:
			 *   - X86_CR4_SMAP is set in CR4
			 *   - A user page is accessed
			 *   - The access is not a fetch
			 *   - The access is supervisor mode
			 *   - If implicit supervisor access or X86_EFLAGS_AC is clear
			 *
			 * Here, we cover the first four conditions.
			 * The fifth is computed dynamically in permission_fault();
			 * PFERR_RSVD_MASK bit will be set in PFEC if the access is
			 * *not* subject to SMAP restrictions.
			 */
			if (cr4_smap)
				smapf = (pfec & (PFERR_RSVD_MASK|PFERR_FETCH_MASK)) ? 0 : kf;
		}

		mmu->permissions[byte] = ff | uf | wf | smepf | smapf;
	}
}

/*
* PKU is an additional mechanism by which the paging controls access to
* user-mode addresses based on the value in the PKRU register.  Protection
* key violations are reported through a bit in the page fault error code.
* Unlike other bits of the error code, the PK bit is not known at the
* call site of e.g. gva_to_gpa; it must be computed directly in
* permission_fault based on two bits of PKRU, on some machine state (CR4,
* CR0, EFER, CPL), and on other bits of the error code and the page tables.
*
* In particular the following conditions come from the error code, the
* page tables and the machine state:
* - PK is always zero unless CR4.PKE=1 and EFER.LMA=1
* - PK is always zero if RSVD=1 (reserved bit set) or F=1 (instruction fetch)
* - PK is always zero if U=0 in the page tables
* - PKRU.WD is ignored if CR0.WP=0 and the access is a supervisor access.
*
* The PKRU bitmask caches the result of these four conditions.  The error
* code (minus the P bit) and the page table's U bit form an index into the
* PKRU bitmask.  Two bits of the PKRU bitmask are then extracted and ANDed
* with the two bits of the PKRU register corresponding to the protection key.
* For the first three conditions above the bits will be 00, thus masking
* away both AD and WD.  For all reads or if the last condition holds, WD
* only will be masked away.
*/
static void update_pkru_bitmask(struct kvm_mmu *mmu)
{
	unsigned bit;
	bool wp;

	mmu->pkru_mask = 0;

	if (!is_cr4_pke(mmu))
		return;

	wp = is_cr0_wp(mmu);

	for (bit = 0; bit < ARRAY_SIZE(mmu->permissions); ++bit) {
		unsigned pfec, pkey_bits;
		bool check_pkey, check_write, ff, uf, wf, pte_user;

		pfec = bit << 1;
		ff = pfec & PFERR_FETCH_MASK;
		uf = pfec & PFERR_USER_MASK;
		wf = pfec & PFERR_WRITE_MASK;

		/* PFEC.RSVD is replaced by ACC_USER_MASK. */
		pte_user = pfec & PFERR_RSVD_MASK;

		/*
		 * Only need to check the access which is not an
		 * instruction fetch and is to a user page.
		 */
		check_pkey = (!ff && pte_user);
		/*
		 * write access is controlled by PKRU if it is a
		 * user access or CR0.WP = 1.
		 */
		check_write = check_pkey && wf && (uf || wp);

		/* PKRU.AD stops both read and write access. */
		pkey_bits = !!check_pkey;
		/* PKRU.WD stops write access. */
		pkey_bits |= (!!check_write) << 1;

		mmu->pkru_mask |= (pkey_bits & 3) << pfec;
	}
}

static void reset_guest_paging_metadata(struct kvm_vcpu *vcpu,
					struct kvm_mmu *mmu)
{
	if (!is_cr0_pg(mmu))
		return;

	reset_guest_rsvds_bits_mask(vcpu, mmu);
	update_permission_bitmask(mmu, false);
	update_pkru_bitmask(mmu);
}

static void paging64_init_context(struct kvm_mmu *context)
{
	context->page_fault = paging64_page_fault;
	context->gva_to_gpa = paging64_gva_to_gpa;
	context->sync_page = paging64_sync_page;
	context->invlpg = paging64_invlpg;
}

static void paging32_init_context(struct kvm_mmu *context)
{
	context->page_fault = paging32_page_fault;
	context->gva_to_gpa = paging32_gva_to_gpa;
	context->sync_page = paging32_sync_page;
	context->invlpg = paging32_invlpg;
}

static union kvm_cpu_role
kvm_calc_cpu_role(struct kvm_vcpu *vcpu, const struct kvm_mmu_role_regs *regs)
{
	union kvm_cpu_role role = {0};

	role.base.access = ACC_ALL;
	role.base.smm = is_smm(vcpu);
	role.base.guest_mode = is_guest_mode(vcpu);
	role.ext.valid = 1;

	if (!____is_cr0_pg(regs)) {
		role.base.direct = 1;
		return role;
	}

	role.base.efer_nx = ____is_efer_nx(regs);
	role.base.cr0_wp = ____is_cr0_wp(regs);
	role.base.smep_andnot_wp = ____is_cr4_smep(regs) && !____is_cr0_wp(regs);
	role.base.smap_andnot_wp = ____is_cr4_smap(regs) && !____is_cr0_wp(regs);
	role.base.has_4_byte_gpte = !____is_cr4_pae(regs);

	if (____is_efer_lma(regs))
		role.base.level = ____is_cr4_la57(regs) ? PT64_ROOT_5LEVEL
							: PT64_ROOT_4LEVEL;
	else if (____is_cr4_pae(regs))
		role.base.level = PT32E_ROOT_LEVEL;
	else
		role.base.level = PT32_ROOT_LEVEL;

	role.ext.cr4_smep = ____is_cr4_smep(regs);
	role.ext.cr4_smap = ____is_cr4_smap(regs);
	role.ext.cr4_pse = ____is_cr4_pse(regs);

	/* PKEY and LA57 are active iff long mode is active. */
	role.ext.cr4_pke = ____is_efer_lma(regs) && ____is_cr4_pke(regs);
	role.ext.cr4_la57 = ____is_efer_lma(regs) && ____is_cr4_la57(regs);
	role.ext.efer_lma = ____is_efer_lma(regs);
	return role;
}

static inline int kvm_mmu_get_tdp_level(struct kvm_vcpu *vcpu)
{
	/* tdp_root_level is architecture forced level, use it if nonzero */
	if (tdp_root_level)
		return tdp_root_level;

	/* Use 5-level TDP if and only if it's useful/necessary. */
	if (max_tdp_level == 5 && cpuid_maxphyaddr(vcpu) <= 48)
		return 4;

	return max_tdp_level;
}

static union kvm_mmu_page_role
kvm_calc_tdp_mmu_root_page_role(struct kvm_vcpu *vcpu,
				union kvm_cpu_role cpu_role)
{
	union kvm_mmu_page_role role = {0};

	role.access = ACC_ALL;
	role.cr0_wp = true;
	role.efer_nx = true;
	role.smm = cpu_role.base.smm;
	role.guest_mode = cpu_role.base.guest_mode;
	role.ad_disabled = !kvm_ad_enabled();
	role.level = kvm_mmu_get_tdp_level(vcpu);
	role.direct = true;
	role.has_4_byte_gpte = false;

	return role;
}

static void init_kvm_tdp_mmu(struct kvm_vcpu *vcpu,
			     union kvm_cpu_role cpu_role)
{
	struct kvm_mmu *context = &vcpu->arch.root_mmu;
	union kvm_mmu_page_role root_role = kvm_calc_tdp_mmu_root_page_role(vcpu, cpu_role);

	if (cpu_role.as_u64 == context->cpu_role.as_u64 &&
	    root_role.word == context->root_role.word)
		return;

	context->cpu_role.as_u64 = cpu_role.as_u64;
	context->root_role.word = root_role.word;
	context->page_fault = kvm_tdp_page_fault;
	context->sync_page = nonpaging_sync_page;
	context->invlpg = NULL;
	context->get_guest_pgd = get_cr3;
	context->get_pdptr = kvm_pdptr_read;
	context->inject_page_fault = kvm_inject_page_fault;

	if (!is_cr0_pg(context))
		context->gva_to_gpa = nonpaging_gva_to_gpa;
	else if (is_cr4_pae(context))
		context->gva_to_gpa = paging64_gva_to_gpa;
	else
		context->gva_to_gpa = paging32_gva_to_gpa;

	reset_guest_paging_metadata(vcpu, context);
	reset_tdp_shadow_zero_bits_mask(context);
}

static void shadow_mmu_init_context(struct kvm_vcpu *vcpu, struct kvm_mmu *context,
				    union kvm_cpu_role cpu_role,
				    union kvm_mmu_page_role root_role)
{
	if (cpu_role.as_u64 == context->cpu_role.as_u64 &&
	    root_role.word == context->root_role.word)
		return;

	context->cpu_role.as_u64 = cpu_role.as_u64;
	context->root_role.word = root_role.word;

	if (!is_cr0_pg(context))
		nonpaging_init_context(context);
	else if (is_cr4_pae(context))
		paging64_init_context(context);
	else
		paging32_init_context(context);

	reset_guest_paging_metadata(vcpu, context);
	reset_shadow_zero_bits_mask(vcpu, context);
}

static void kvm_init_shadow_mmu(struct kvm_vcpu *vcpu,
				union kvm_cpu_role cpu_role)
{
	struct kvm_mmu *context = &vcpu->arch.root_mmu;
	union kvm_mmu_page_role root_role;

	root_role = cpu_role.base;

	/* KVM uses PAE paging whenever the guest isn't using 64-bit paging. */
	root_role.level = max_t(u32, root_role.level, PT32E_ROOT_LEVEL);

	/*
	 * KVM forces EFER.NX=1 when TDP is disabled, reflect it in the MMU role.
	 * KVM uses NX when TDP is disabled to handle a variety of scenarios,
	 * notably for huge SPTEs if iTLB multi-hit mitigation is enabled and
	 * to generate correct permissions for CR0.WP=0/CR4.SMEP=1/EFER.NX=0.
	 * The iTLB multi-hit workaround can be toggled at any time, so assume
	 * NX can be used by any non-nested shadow MMU to avoid having to reset
	 * MMU contexts.
	 */
	root_role.efer_nx = true;

	shadow_mmu_init_context(vcpu, context, cpu_role, root_role);
}

void kvm_init_shadow_npt_mmu(struct kvm_vcpu *vcpu, unsigned long cr0,
			     unsigned long cr4, u64 efer, gpa_t nested_cr3)
{
	struct kvm_mmu *context = &vcpu->arch.guest_mmu;
	struct kvm_mmu_role_regs regs = {
		.cr0 = cr0,
		.cr4 = cr4 & ~X86_CR4_PKE,
		.efer = efer,
	};
	union kvm_cpu_role cpu_role = kvm_calc_cpu_role(vcpu, &regs);
	union kvm_mmu_page_role root_role;

	/* NPT requires CR0.PG=1. */
	WARN_ON_ONCE(cpu_role.base.direct);

	root_role = cpu_role.base;
	root_role.level = kvm_mmu_get_tdp_level(vcpu);
	if (root_role.level == PT64_ROOT_5LEVEL &&
	    cpu_role.base.level == PT64_ROOT_4LEVEL)
		root_role.passthrough = 1;

	shadow_mmu_init_context(vcpu, context, cpu_role, root_role);
	kvm_mmu_new_pgd(vcpu, nested_cr3);
}
EXPORT_SYMBOL_GPL(kvm_init_shadow_npt_mmu);

static union kvm_cpu_role
kvm_calc_shadow_ept_root_page_role(struct kvm_vcpu *vcpu, bool accessed_dirty,
				   bool execonly, u8 level)
{
	union kvm_cpu_role role = {0};

	/*
	 * KVM does not support SMM transfer monitors, and consequently does not
	 * support the "entry to SMM" control either.  role.base.smm is always 0.
	 */
	WARN_ON_ONCE(is_smm(vcpu));
	role.base.level = level;
	role.base.has_4_byte_gpte = false;
	role.base.direct = false;
	role.base.ad_disabled = !accessed_dirty;
	role.base.guest_mode = true;
	role.base.access = ACC_ALL;

	role.ext.word = 0;
	role.ext.execonly = execonly;
	role.ext.valid = 1;

	return role;
}

void kvm_init_shadow_ept_mmu(struct kvm_vcpu *vcpu, bool execonly,
			     int huge_page_level, bool accessed_dirty,
			     gpa_t new_eptp)
{
	struct kvm_mmu *context = &vcpu->arch.guest_mmu;
	u8 level = vmx_eptp_page_walk_level(new_eptp);
	union kvm_cpu_role new_mode =
		kvm_calc_shadow_ept_root_page_role(vcpu, accessed_dirty,
						   execonly, level);

	if (new_mode.as_u64 != context->cpu_role.as_u64) {
		/* EPT, and thus nested EPT, does not consume CR0, CR4, nor EFER. */
		context->cpu_role.as_u64 = new_mode.as_u64;
		context->root_role.word = new_mode.base.word;

		context->page_fault = ept_page_fault;
		context->gva_to_gpa = ept_gva_to_gpa;
		context->sync_page = ept_sync_page;
		context->invlpg = ept_invlpg;

		update_permission_bitmask(context, true);
		context->pkru_mask = 0;
		reset_rsvds_bits_mask_ept(vcpu, context, execonly, huge_page_level);
		reset_ept_shadow_zero_bits_mask(context, execonly);
	}

	kvm_mmu_new_pgd(vcpu, new_eptp);
}
EXPORT_SYMBOL_GPL(kvm_init_shadow_ept_mmu);

static void init_kvm_softmmu(struct kvm_vcpu *vcpu,
			     union kvm_cpu_role cpu_role)
{
	struct kvm_mmu *context = &vcpu->arch.root_mmu;

	kvm_init_shadow_mmu(vcpu, cpu_role);

	context->get_guest_pgd     = get_cr3;
	context->get_pdptr         = kvm_pdptr_read;
	context->inject_page_fault = kvm_inject_page_fault;
}

static void init_kvm_nested_mmu(struct kvm_vcpu *vcpu,
				union kvm_cpu_role new_mode)
{
	struct kvm_mmu *g_context = &vcpu->arch.nested_mmu;

	if (new_mode.as_u64 == g_context->cpu_role.as_u64)
		return;

	g_context->cpu_role.as_u64   = new_mode.as_u64;
	g_context->get_guest_pgd     = get_cr3;
	g_context->get_pdptr         = kvm_pdptr_read;
	g_context->inject_page_fault = kvm_inject_page_fault;

	/*
	 * L2 page tables are never shadowed, so there is no need to sync
	 * SPTEs.
	 */
	g_context->invlpg            = NULL;

	/*
	 * Note that arch.mmu->gva_to_gpa translates l2_gpa to l1_gpa using
	 * L1's nested page tables (e.g. EPT12). The nested translation
	 * of l2_gva to l1_gpa is done by arch.nested_mmu.gva_to_gpa using
	 * L2's page tables as the first level of translation and L1's
	 * nested page tables as the second level of translation. Basically
	 * the gva_to_gpa functions between mmu and nested_mmu are swapped.
	 */
	if (!is_paging(vcpu))
		g_context->gva_to_gpa = nonpaging_gva_to_gpa;
	else if (is_long_mode(vcpu))
		g_context->gva_to_gpa = paging64_gva_to_gpa;
	else if (is_pae(vcpu))
		g_context->gva_to_gpa = paging64_gva_to_gpa;
	else
		g_context->gva_to_gpa = paging32_gva_to_gpa;

	reset_guest_paging_metadata(vcpu, g_context);
}

void kvm_init_mmu(struct kvm_vcpu *vcpu)
{
	struct kvm_mmu_role_regs regs = vcpu_to_role_regs(vcpu);
	union kvm_cpu_role cpu_role = kvm_calc_cpu_role(vcpu, &regs);

	if (mmu_is_nested(vcpu))
		init_kvm_nested_mmu(vcpu, cpu_role);
	else if (tdp_enabled)
		init_kvm_tdp_mmu(vcpu, cpu_role);
	else
		init_kvm_softmmu(vcpu, cpu_role);
}
EXPORT_SYMBOL_GPL(kvm_init_mmu);

void kvm_mmu_after_set_cpuid(struct kvm_vcpu *vcpu)
{
	/*
	 * Invalidate all MMU roles to force them to reinitialize as CPUID
	 * information is factored into reserved bit calculations.
	 *
	 * Correctly handling multiple vCPU models with respect to paging and
	 * physical address properties) in a single VM would require tracking
	 * all relevant CPUID information in kvm_mmu_page_role. That is very
	 * undesirable as it would increase the memory requirements for
	 * gfn_track (see struct kvm_mmu_page_role comments).  For now that
	 * problem is swept under the rug; KVM's CPUID API is horrific and
	 * it's all but impossible to solve it without introducing a new API.
	 */
	vcpu->arch.root_mmu.root_role.word = 0;
	vcpu->arch.guest_mmu.root_role.word = 0;
	vcpu->arch.nested_mmu.root_role.word = 0;
	vcpu->arch.root_mmu.cpu_role.ext.valid = 0;
	vcpu->arch.guest_mmu.cpu_role.ext.valid = 0;
	vcpu->arch.nested_mmu.cpu_role.ext.valid = 0;
	kvm_mmu_reset_context(vcpu);

	/*
	 * Changing guest CPUID after KVM_RUN is forbidden, see the comment in
	 * kvm_arch_vcpu_ioctl().
	 */
	KVM_BUG_ON(vcpu->arch.last_vmentry_cpu != -1, vcpu->kvm);
}

void kvm_mmu_reset_context(struct kvm_vcpu *vcpu)
{
	kvm_mmu_unload(vcpu);
	kvm_init_mmu(vcpu);
}
EXPORT_SYMBOL_GPL(kvm_mmu_reset_context);

int kvm_mmu_load(struct kvm_vcpu *vcpu)
{
	int r;

	r = mmu_topup_memory_caches(vcpu, !vcpu->arch.mmu->root_role.direct);
	if (r)
		goto out;
	r = mmu_alloc_special_roots(vcpu);
	if (r)
		goto out;
	if (vcpu->arch.mmu->root_role.direct)
		r = mmu_alloc_direct_roots(vcpu);
	else
		r = mmu_alloc_shadow_roots(vcpu);
	if (r)
		goto out;

	kvm_mmu_sync_roots(vcpu);

	kvm_mmu_load_pgd(vcpu);

	/*
	 * Flush any TLB entries for the new root, the provenance of the root
	 * is unknown.  Even if KVM ensures there are no stale TLB entries
	 * for a freed root, in theory another hypervisor could have left
	 * stale entries.  Flushing on alloc also allows KVM to skip the TLB
	 * flush when freeing a root (see kvm_tdp_mmu_put_root()).
	 */
	static_call(kvm_x86_flush_tlb_current)(vcpu);
out:
	return r;
}

void kvm_mmu_unload(struct kvm_vcpu *vcpu)
{
	struct kvm *kvm = vcpu->kvm;

	kvm_mmu_free_roots(kvm, &vcpu->arch.root_mmu, KVM_MMU_ROOTS_ALL);
	WARN_ON(VALID_PAGE(vcpu->arch.root_mmu.root.hpa));
	kvm_mmu_free_roots(kvm, &vcpu->arch.guest_mmu, KVM_MMU_ROOTS_ALL);
	WARN_ON(VALID_PAGE(vcpu->arch.guest_mmu.root.hpa));
	vcpu_clear_mmio_info(vcpu, MMIO_GVA_ANY);
}

static bool is_obsolete_root(struct kvm *kvm, hpa_t root_hpa)
{
	struct kvm_mmu_page *sp;

	if (!VALID_PAGE(root_hpa))
		return false;

	/*
	 * When freeing obsolete roots, treat roots as obsolete if they don't
	 * have an associated shadow page.  This does mean KVM will get false
	 * positives and free roots that don't strictly need to be freed, but
	 * such false positives are relatively rare:
	 *
	 *  (a) only PAE paging and nested NPT has roots without shadow pages
	 *  (b) remote reloads due to a memslot update obsoletes _all_ roots
	 *  (c) KVM doesn't track previous roots for PAE paging, and the guest
	 *      is unlikely to zap an in-use PGD.
	 */
	sp = to_shadow_page(root_hpa);
	return !sp || is_obsolete_sp(kvm, sp);
}

static void __kvm_mmu_free_obsolete_roots(struct kvm *kvm, struct kvm_mmu *mmu)
{
	unsigned long roots_to_free = 0;
	int i;

	if (is_obsolete_root(kvm, mmu->root.hpa))
		roots_to_free |= KVM_MMU_ROOT_CURRENT;

	for (i = 0; i < KVM_MMU_NUM_PREV_ROOTS; i++) {
		if (is_obsolete_root(kvm, mmu->prev_roots[i].hpa))
			roots_to_free |= KVM_MMU_ROOT_PREVIOUS(i);
	}

	if (roots_to_free)
		kvm_mmu_free_roots(kvm, mmu, roots_to_free);
}

void kvm_mmu_free_obsolete_roots(struct kvm_vcpu *vcpu)
{
	__kvm_mmu_free_obsolete_roots(vcpu->kvm, &vcpu->arch.root_mmu);
	__kvm_mmu_free_obsolete_roots(vcpu->kvm, &vcpu->arch.guest_mmu);
}

static u64 mmu_pte_write_fetch_gpte(struct kvm_vcpu *vcpu, gpa_t *gpa,
				    int *bytes)
{
	u64 gentry = 0;
	int r;

	/*
	 * Assume that the pte write on a page table of the same type
	 * as the current vcpu paging mode since we update the sptes only
	 * when they have the same mode.
	 */
	if (is_pae(vcpu) && *bytes == 4) {
		/* Handle a 32-bit guest writing two halves of a 64-bit gpte */
		*gpa &= ~(gpa_t)7;
		*bytes = 8;
	}

	if (*bytes == 4 || *bytes == 8) {
		r = kvm_vcpu_read_guest_atomic(vcpu, *gpa, &gentry, *bytes);
		if (r)
			gentry = 0;
	}

	return gentry;
}

/*
 * If we're seeing too many writes to a page, it may no longer be a page table,
 * or we may be forking, in which case it is better to unmap the page.
 */
static bool detect_write_flooding(struct kvm_mmu_page *sp)
{
	/*
	 * Skip write-flooding detected for the sp whose level is 1, because
	 * it can become unsync, then the guest page is not write-protected.
	 */
	if (sp->role.level == PG_LEVEL_4K)
		return false;

	atomic_inc(&sp->write_flooding_count);
	return atomic_read(&sp->write_flooding_count) >= 3;
}

/*
 * Misaligned accesses are too much trouble to fix up; also, they usually
 * indicate a page is not used as a page table.
 */
static bool detect_write_misaligned(struct kvm_mmu_page *sp, gpa_t gpa,
				    int bytes)
{
	unsigned offset, pte_size, misaligned;

	pgprintk("misaligned: gpa %llx bytes %d role %x\n",
		 gpa, bytes, sp->role.word);

	offset = offset_in_page(gpa);
	pte_size = sp->role.has_4_byte_gpte ? 4 : 8;

	/*
	 * Sometimes, the OS only writes the last one bytes to update status
	 * bits, for example, in linux, andb instruction is used in clear_bit().
	 */
	if (!(offset & (pte_size - 1)) && bytes == 1)
		return false;

	misaligned = (offset ^ (offset + bytes - 1)) & ~(pte_size - 1);
	misaligned |= bytes < 4;

	return misaligned;
}

static u64 *get_written_sptes(struct kvm_mmu_page *sp, gpa_t gpa, int *nspte)
{
	unsigned page_offset, quadrant;
	u64 *spte;
	int level;

	page_offset = offset_in_page(gpa);
	level = sp->role.level;
	*nspte = 1;
	if (sp->role.has_4_byte_gpte) {
		page_offset <<= 1;	/* 32->64 */
		/*
		 * A 32-bit pde maps 4MB while the shadow pdes map
		 * only 2MB.  So we need to double the offset again
		 * and zap two pdes instead of one.
		 */
		if (level == PT32_ROOT_LEVEL) {
			page_offset &= ~7; /* kill rounding error */
			page_offset <<= 1;
			*nspte = 2;
		}
		quadrant = page_offset >> PAGE_SHIFT;
		page_offset &= ~PAGE_MASK;
		if (quadrant != sp->role.quadrant)
			return NULL;
	}

	spte = &sp->spt[page_offset / sizeof(*spte)];
	return spte;
}

static void kvm_mmu_pte_write(struct kvm_vcpu *vcpu, gpa_t gpa,
			      const u8 *new, int bytes,
			      struct kvm_page_track_notifier_node *node)
{
	gfn_t gfn = gpa >> PAGE_SHIFT;
	struct kvm_mmu_page *sp;
	LIST_HEAD(invalid_list);
	u64 entry, gentry, *spte;
	int npte;
	bool flush = false;

	/*
	 * If we don't have indirect shadow pages, it means no page is
	 * write-protected, so we can exit simply.
	 */
	if (!READ_ONCE(vcpu->kvm->arch.indirect_shadow_pages))
		return;

	pgprintk("%s: gpa %llx bytes %d\n", __func__, gpa, bytes);

	write_lock(&vcpu->kvm->mmu_lock);

	gentry = mmu_pte_write_fetch_gpte(vcpu, &gpa, &bytes);

	++vcpu->kvm->stat.mmu_pte_write;

	for_each_gfn_valid_sp_with_gptes(vcpu->kvm, sp, gfn) {
		if (detect_write_misaligned(sp, gpa, bytes) ||
		      detect_write_flooding(sp)) {
			kvm_mmu_prepare_zap_page(vcpu->kvm, sp, &invalid_list);
			++vcpu->kvm->stat.mmu_flooded;
			continue;
		}

		spte = get_written_sptes(sp, gpa, &npte);
		if (!spte)
			continue;

		while (npte--) {
			entry = *spte;
			mmu_page_zap_pte(vcpu->kvm, sp, spte, NULL);
			if (gentry && sp->role.level != PG_LEVEL_4K)
				++vcpu->kvm->stat.mmu_pde_zapped;
			if (is_shadow_present_pte(entry))
				flush = true;
			++spte;
		}
	}
	kvm_mmu_remote_flush_or_zap(vcpu->kvm, &invalid_list, flush);
	write_unlock(&vcpu->kvm->mmu_lock);
}

int noinline kvm_mmu_page_fault(struct kvm_vcpu *vcpu, gpa_t cr2_or_gpa, u64 error_code,
		       void *insn, int insn_len)
{
	int r, emulation_type = EMULTYPE_PF;
	bool direct = vcpu->arch.mmu->root_role.direct;

	if (WARN_ON(!VALID_PAGE(vcpu->arch.mmu->root.hpa)))
		return RET_PF_RETRY;

	r = RET_PF_INVALID;
	if (unlikely(error_code & PFERR_RSVD_MASK)) {
		r = handle_mmio_page_fault(vcpu, cr2_or_gpa, direct);
		if (r == RET_PF_EMULATE)
			goto emulate;
	}

	if (r == RET_PF_INVALID) {
		r = kvm_mmu_do_page_fault(vcpu, cr2_or_gpa,
					  lower_32_bits(error_code), false);
		if (KVM_BUG_ON(r == RET_PF_INVALID, vcpu->kvm))
			return -EIO;
	}

	if (r < 0)
		return r;
	if (r != RET_PF_EMULATE)
		return 1;

	/*
	 * Before emulating the instruction, check if the error code
	 * was due to a RO violation while translating the guest page.
	 * This can occur when using nested virtualization with nested
	 * paging in both guests. If true, we simply unprotect the page
	 * and resume the guest.
	 */
	if (vcpu->arch.mmu->root_role.direct &&
	    (error_code & PFERR_NESTED_GUEST_PAGE) == PFERR_NESTED_GUEST_PAGE) {
		kvm_mmu_unprotect_page(vcpu->kvm, gpa_to_gfn(cr2_or_gpa));
		return 1;
	}

	/*
	 * vcpu->arch.mmu.page_fault returned RET_PF_EMULATE, but we can still
	 * optimistically try to just unprotect the page and let the processor
	 * re-execute the instruction that caused the page fault.  Do not allow
	 * retrying MMIO emulation, as it's not only pointless but could also
	 * cause us to enter an infinite loop because the processor will keep
	 * faulting on the non-existent MMIO address.  Retrying an instruction
	 * from a nested guest is also pointless and dangerous as we are only
	 * explicitly shadowing L1's page tables, i.e. unprotecting something
	 * for L1 isn't going to magically fix whatever issue cause L2 to fail.
	 */
	if (!mmio_info_in_cache(vcpu, cr2_or_gpa, direct) && !is_guest_mode(vcpu))
		emulation_type |= EMULTYPE_ALLOW_RETRY_PF;
emulate:
	return x86_emulate_instruction(vcpu, cr2_or_gpa, emulation_type, insn,
				       insn_len);
}
EXPORT_SYMBOL_GPL(kvm_mmu_page_fault);

void kvm_mmu_invalidate_gva(struct kvm_vcpu *vcpu, struct kvm_mmu *mmu,
			    gva_t gva, hpa_t root_hpa)
{
	int i;

	/* It's actually a GPA for vcpu->arch.guest_mmu.  */
	if (mmu != &vcpu->arch.guest_mmu) {
		/* INVLPG on a non-canonical address is a NOP according to the SDM.  */
		if (is_noncanonical_address(gva, vcpu))
			return;

		static_call(kvm_x86_flush_tlb_gva)(vcpu, gva);
	}

	if (!mmu->invlpg)
		return;

	if (root_hpa == INVALID_PAGE) {
		mmu->invlpg(vcpu, gva, mmu->root.hpa);

		/*
		 * INVLPG is required to invalidate any global mappings for the VA,
		 * irrespective of PCID. Since it would take us roughly similar amount
		 * of work to determine whether any of the prev_root mappings of the VA
		 * is marked global, or to just sync it blindly, so we might as well
		 * just always sync it.
		 *
		 * Mappings not reachable via the current cr3 or the prev_roots will be
		 * synced when switching to that cr3, so nothing needs to be done here
		 * for them.
		 */
		for (i = 0; i < KVM_MMU_NUM_PREV_ROOTS; i++)
			if (VALID_PAGE(mmu->prev_roots[i].hpa))
				mmu->invlpg(vcpu, gva, mmu->prev_roots[i].hpa);
	} else {
		mmu->invlpg(vcpu, gva, root_hpa);
	}
}

void kvm_mmu_invlpg(struct kvm_vcpu *vcpu, gva_t gva)
{
	kvm_mmu_invalidate_gva(vcpu, vcpu->arch.walk_mmu, gva, INVALID_PAGE);
	++vcpu->stat.invlpg;
}
EXPORT_SYMBOL_GPL(kvm_mmu_invlpg);


void kvm_mmu_invpcid_gva(struct kvm_vcpu *vcpu, gva_t gva, unsigned long pcid)
{
	struct kvm_mmu *mmu = vcpu->arch.mmu;
	bool tlb_flush = false;
	uint i;

	if (pcid == kvm_get_active_pcid(vcpu)) {
		if (mmu->invlpg)
			mmu->invlpg(vcpu, gva, mmu->root.hpa);
		tlb_flush = true;
	}

	for (i = 0; i < KVM_MMU_NUM_PREV_ROOTS; i++) {
		if (VALID_PAGE(mmu->prev_roots[i].hpa) &&
		    pcid == kvm_get_pcid(vcpu, mmu->prev_roots[i].pgd)) {
			if (mmu->invlpg)
				mmu->invlpg(vcpu, gva, mmu->prev_roots[i].hpa);
			tlb_flush = true;
		}
	}

	if (tlb_flush)
		static_call(kvm_x86_flush_tlb_gva)(vcpu, gva);

	++vcpu->stat.invlpg;

	/*
	 * Mappings not reachable via the current cr3 or the prev_roots will be
	 * synced when switching to that cr3, so nothing needs to be done here
	 * for them.
	 */
}

void kvm_configure_mmu(bool enable_tdp, int tdp_forced_root_level,
		       int tdp_max_root_level, int tdp_huge_page_level)
{
	tdp_enabled = enable_tdp;
	tdp_root_level = tdp_forced_root_level;
	max_tdp_level = tdp_max_root_level;

	/*
	 * max_huge_page_level reflects KVM's MMU capabilities irrespective
	 * of kernel support, e.g. KVM may be capable of using 1GB pages when
	 * the kernel is not.  But, KVM never creates a page size greater than
	 * what is used by the kernel for any given HVA, i.e. the kernel's
	 * capabilities are ultimately consulted by kvm_mmu_hugepage_adjust().
	 */
	if (tdp_enabled)
		max_huge_page_level = tdp_huge_page_level;
	else if (boot_cpu_has(X86_FEATURE_GBPAGES))
		max_huge_page_level = PG_LEVEL_1G;
	else
		max_huge_page_level = PG_LEVEL_2M;
}
EXPORT_SYMBOL_GPL(kvm_configure_mmu);

/* The return value indicates if tlb flush on all vcpus is needed. */
typedef bool (*slot_level_handler) (struct kvm *kvm,
				    struct kvm_rmap_head *rmap_head,
				    const struct kvm_memory_slot *slot);

/* The caller should hold mmu-lock before calling this function. */
static __always_inline bool
slot_handle_level_range(struct kvm *kvm, const struct kvm_memory_slot *memslot,
			slot_level_handler fn, int start_level, int end_level,
			gfn_t start_gfn, gfn_t end_gfn, bool flush_on_yield,
			bool flush)
{
	struct slot_rmap_walk_iterator iterator;

	for_each_slot_rmap_range(memslot, start_level, end_level, start_gfn,
			end_gfn, &iterator) {
		if (iterator.rmap)
			flush |= fn(kvm, iterator.rmap, memslot);

		if (need_resched() || rwlock_needbreak(&kvm->mmu_lock)) {
			if (flush && flush_on_yield) {
				kvm_flush_remote_tlbs_with_address(kvm,
						start_gfn,
						iterator.gfn - start_gfn + 1);
				flush = false;
			}
			cond_resched_rwlock_write(&kvm->mmu_lock);
		}
	}

	return flush;
}

static __always_inline bool
slot_handle_level(struct kvm *kvm, const struct kvm_memory_slot *memslot,
		  slot_level_handler fn, int start_level, int end_level,
		  bool flush_on_yield)
{
	return slot_handle_level_range(kvm, memslot, fn, start_level,
			end_level, memslot->base_gfn,
			memslot->base_gfn + memslot->npages - 1,
			flush_on_yield, false);
}

static __always_inline bool
slot_handle_level_4k(struct kvm *kvm, const struct kvm_memory_slot *memslot,
		     slot_level_handler fn, bool flush_on_yield)
{
	return slot_handle_level(kvm, memslot, fn, PG_LEVEL_4K,
				 PG_LEVEL_4K, flush_on_yield);
}

static void free_mmu_pages(struct kvm_mmu *mmu)
{
	if (!tdp_enabled && mmu->pae_root)
		set_memory_encrypted((unsigned long)mmu->pae_root, 1);
	free_page((unsigned long)mmu->pae_root);
	free_page((unsigned long)mmu->pml4_root);
	free_page((unsigned long)mmu->pml5_root);
}

static int __kvm_mmu_create(struct kvm_vcpu *vcpu, struct kvm_mmu *mmu)
{
	struct page *page;
	int i;

	mmu->root.hpa = INVALID_PAGE;
	mmu->root.pgd = 0;
	for (i = 0; i < KVM_MMU_NUM_PREV_ROOTS; i++)
		mmu->prev_roots[i] = KVM_MMU_ROOT_INFO_INVALID;

	/* vcpu->arch.guest_mmu isn't used when !tdp_enabled. */
	if (!tdp_enabled && mmu == &vcpu->arch.guest_mmu)
		return 0;

	/*
	 * When using PAE paging, the four PDPTEs are treated as 'root' pages,
	 * while the PDP table is a per-vCPU construct that's allocated at MMU
	 * creation.  When emulating 32-bit mode, cr3 is only 32 bits even on
	 * x86_64.  Therefore we need to allocate the PDP table in the first
	 * 4GB of memory, which happens to fit the DMA32 zone.  TDP paging
	 * generally doesn't use PAE paging and can skip allocating the PDP
	 * table.  The main exception, handled here, is SVM's 32-bit NPT.  The
	 * other exception is for shadowing L1's 32-bit or PAE NPT on 64-bit
	 * KVM; that horror is handled on-demand by mmu_alloc_special_roots().
	 */
	if (tdp_enabled && kvm_mmu_get_tdp_level(vcpu) > PT32E_ROOT_LEVEL)
		return 0;

	page = alloc_page(GFP_KERNEL_ACCOUNT | __GFP_DMA32);
	if (!page)
		return -ENOMEM;

	mmu->pae_root = page_address(page);

	/*
	 * CR3 is only 32 bits when PAE paging is used, thus it's impossible to
	 * get the CPU to treat the PDPTEs as encrypted.  Decrypt the page so
	 * that KVM's writes and the CPU's reads get along.  Note, this is
	 * only necessary when using shadow paging, as 64-bit NPT can get at
	 * the C-bit even when shadowing 32-bit NPT, and SME isn't supported
	 * by 32-bit kernels (when KVM itself uses 32-bit NPT).
	 */
	if (!tdp_enabled)
		set_memory_decrypted((unsigned long)mmu->pae_root, 1);
	else
		WARN_ON_ONCE(shadow_me_value);

	for (i = 0; i < 4; ++i)
		mmu->pae_root[i] = INVALID_PAE_ROOT;

	return 0;
}

int kvm_mmu_create(struct kvm_vcpu *vcpu)
{
	int ret;

	vcpu->arch.mmu_pte_list_desc_cache.kmem_cache = pte_list_desc_cache;
	vcpu->arch.mmu_pte_list_desc_cache.gfp_zero = __GFP_ZERO;

	vcpu->arch.mmu_page_header_cache.kmem_cache = mmu_page_header_cache;
	vcpu->arch.mmu_page_header_cache.gfp_zero = __GFP_ZERO;

	vcpu->arch.mmu_shadow_page_cache.gfp_zero = __GFP_ZERO;

	vcpu->arch.mmu = &vcpu->arch.root_mmu;
	vcpu->arch.walk_mmu = &vcpu->arch.root_mmu;

	ret = __kvm_mmu_create(vcpu, &vcpu->arch.guest_mmu);
	if (ret)
		return ret;

	ret = __kvm_mmu_create(vcpu, &vcpu->arch.root_mmu);
	if (ret)
		goto fail_allocate_root;

	return ret;
 fail_allocate_root:
	free_mmu_pages(&vcpu->arch.guest_mmu);
	return ret;
}

#define BATCH_ZAP_PAGES	10
static void kvm_zap_obsolete_pages(struct kvm *kvm)
{
	struct kvm_mmu_page *sp, *node;
	int nr_zapped, batch = 0;
	bool unstable;

restart:
	list_for_each_entry_safe_reverse(sp, node,
	      &kvm->arch.active_mmu_pages, link) {
		/*
		 * No obsolete valid page exists before a newly created page
		 * since active_mmu_pages is a FIFO list.
		 */
		if (!is_obsolete_sp(kvm, sp))
			break;

		/*
		 * Invalid pages should never land back on the list of active
		 * pages.  Skip the bogus page, otherwise we'll get stuck in an
		 * infinite loop if the page gets put back on the list (again).
		 */
		if (WARN_ON(sp->role.invalid))
			continue;

		/*
		 * No need to flush the TLB since we're only zapping shadow
		 * pages with an obsolete generation number and all vCPUS have
		 * loaded a new root, i.e. the shadow pages being zapped cannot
		 * be in active use by the guest.
		 */
		if (batch >= BATCH_ZAP_PAGES &&
		    cond_resched_rwlock_write(&kvm->mmu_lock)) {
			batch = 0;
			goto restart;
		}

		unstable = __kvm_mmu_prepare_zap_page(kvm, sp,
				&kvm->arch.zapped_obsolete_pages, &nr_zapped);
		batch += nr_zapped;

		if (unstable)
			goto restart;
	}

	/*
	 * Kick all vCPUs (via remote TLB flush) before freeing the page tables
	 * to ensure KVM is not in the middle of a lockless shadow page table
	 * walk, which may reference the pages.  The remote TLB flush itself is
	 * not required and is simply a convenient way to kick vCPUs as needed.
	 * KVM performs a local TLB flush when allocating a new root (see
	 * kvm_mmu_load()), and the reload in the caller ensure no vCPUs are
	 * running with an obsolete MMU.
	 */
	kvm_mmu_commit_zap_page(kvm, &kvm->arch.zapped_obsolete_pages);
}

/*
 * Fast invalidate all shadow pages and use lock-break technique
 * to zap obsolete pages.
 *
 * It's required when memslot is being deleted or VM is being
 * destroyed, in these cases, we should ensure that KVM MMU does
 * not use any resource of the being-deleted slot or all slots
 * after calling the function.
 */
static void kvm_mmu_zap_all_fast(struct kvm *kvm)
{
	lockdep_assert_held(&kvm->slots_lock);

	write_lock(&kvm->mmu_lock);
	trace_kvm_mmu_zap_all_fast(kvm);

	/*
	 * Toggle mmu_valid_gen between '0' and '1'.  Because slots_lock is
	 * held for the entire duration of zapping obsolete pages, it's
	 * impossible for there to be multiple invalid generations associated
	 * with *valid* shadow pages at any given time, i.e. there is exactly
	 * one valid generation and (at most) one invalid generation.
	 */
	kvm->arch.mmu_valid_gen = kvm->arch.mmu_valid_gen ? 0 : 1;

	/*
	 * In order to ensure all vCPUs drop their soon-to-be invalid roots,
	 * invalidating TDP MMU roots must be done while holding mmu_lock for
	 * write and in the same critical section as making the reload request,
	 * e.g. before kvm_zap_obsolete_pages() could drop mmu_lock and yield.
	 */
	if (is_tdp_mmu_enabled(kvm))
		kvm_tdp_mmu_invalidate_all_roots(kvm);

	/*
	 * Notify all vcpus to reload its shadow page table and flush TLB.
	 * Then all vcpus will switch to new shadow page table with the new
	 * mmu_valid_gen.
	 *
	 * Note: we need to do this under the protection of mmu_lock,
	 * otherwise, vcpu would purge shadow page but miss tlb flush.
	 */
	kvm_make_all_cpus_request(kvm, KVM_REQ_MMU_FREE_OBSOLETE_ROOTS);

	kvm_zap_obsolete_pages(kvm);

	write_unlock(&kvm->mmu_lock);

	/*
	 * Zap the invalidated TDP MMU roots, all SPTEs must be dropped before
	 * returning to the caller, e.g. if the zap is in response to a memslot
	 * deletion, mmu_notifier callbacks will be unable to reach the SPTEs
	 * associated with the deleted memslot once the update completes, and
	 * Deferring the zap until the final reference to the root is put would
	 * lead to use-after-free.
	 */
	if (is_tdp_mmu_enabled(kvm))
		kvm_tdp_mmu_zap_invalidated_roots(kvm);
}

static bool kvm_has_zapped_obsolete_pages(struct kvm *kvm)
{
	return unlikely(!list_empty_careful(&kvm->arch.zapped_obsolete_pages));
}

static void kvm_mmu_invalidate_zap_pages_in_memslot(struct kvm *kvm,
			struct kvm_memory_slot *slot,
			struct kvm_page_track_notifier_node *node)
{
	kvm_mmu_zap_all_fast(kvm);
}

int kvm_mmu_init_vm(struct kvm *kvm)
{
	struct kvm_page_track_notifier_node *node = &kvm->arch.mmu_sp_tracker;
	int r;

	INIT_LIST_HEAD(&kvm->arch.active_mmu_pages);
	INIT_LIST_HEAD(&kvm->arch.zapped_obsolete_pages);
	INIT_LIST_HEAD(&kvm->arch.lpage_disallowed_mmu_pages);
	spin_lock_init(&kvm->arch.mmu_unsync_pages_lock);

	r = kvm_mmu_init_tdp_mmu(kvm);
	if (r < 0)
		return r;

	node->track_write = kvm_mmu_pte_write;
	node->track_flush_slot = kvm_mmu_invalidate_zap_pages_in_memslot;
	kvm_page_track_register_notifier(kvm, node);

	kvm->arch.split_page_header_cache.kmem_cache = mmu_page_header_cache;
	kvm->arch.split_page_header_cache.gfp_zero = __GFP_ZERO;

	kvm->arch.split_shadow_page_cache.gfp_zero = __GFP_ZERO;

	kvm->arch.split_desc_cache.kmem_cache = pte_list_desc_cache;
	kvm->arch.split_desc_cache.gfp_zero = __GFP_ZERO;

	return 0;
}

static void mmu_free_vm_memory_caches(struct kvm *kvm)
{
	kvm_mmu_free_memory_cache(&kvm->arch.split_desc_cache);
	kvm_mmu_free_memory_cache(&kvm->arch.split_page_header_cache);
	kvm_mmu_free_memory_cache(&kvm->arch.split_shadow_page_cache);
}

void kvm_mmu_uninit_vm(struct kvm *kvm)
{
	struct kvm_page_track_notifier_node *node = &kvm->arch.mmu_sp_tracker;

	kvm_page_track_unregister_notifier(kvm, node);

	kvm_mmu_uninit_tdp_mmu(kvm);

	mmu_free_vm_memory_caches(kvm);
}

static bool kvm_rmap_zap_gfn_range(struct kvm *kvm, gfn_t gfn_start, gfn_t gfn_end)
{
	const struct kvm_memory_slot *memslot;
	struct kvm_memslots *slots;
	struct kvm_memslot_iter iter;
	bool flush = false;
	gfn_t start, end;
	int i;

	if (!kvm_memslots_have_rmaps(kvm))
		return flush;

	for (i = 0; i < KVM_ADDRESS_SPACE_NUM; i++) {
		slots = __kvm_memslots(kvm, i);

		kvm_for_each_memslot_in_gfn_range(&iter, slots, gfn_start, gfn_end) {
			memslot = iter.slot;
			start = max(gfn_start, memslot->base_gfn);
			end = min(gfn_end, memslot->base_gfn + memslot->npages);
			if (WARN_ON_ONCE(start >= end))
				continue;

			flush = slot_handle_level_range(kvm, memslot, __kvm_zap_rmap,
							PG_LEVEL_4K, KVM_MAX_HUGEPAGE_LEVEL,
							start, end - 1, true, flush);
		}
	}

	return flush;
}

/*
 * Invalidate (zap) SPTEs that cover GFNs from gfn_start and up to gfn_end
 * (not including it)
 */
void kvm_zap_gfn_range(struct kvm *kvm, gfn_t gfn_start, gfn_t gfn_end)
{
	bool flush;
	int i;

	if (WARN_ON_ONCE(gfn_end <= gfn_start))
		return;

	write_lock(&kvm->mmu_lock);

	kvm_mmu_invalidate_begin(kvm, 0, -1ul);

	flush = kvm_rmap_zap_gfn_range(kvm, gfn_start, gfn_end);

	if (is_tdp_mmu_enabled(kvm)) {
		for (i = 0; i < KVM_ADDRESS_SPACE_NUM; i++)
			flush = kvm_tdp_mmu_zap_leafs(kvm, i, gfn_start,
						      gfn_end, true, flush);
	}

	if (flush)
		kvm_flush_remote_tlbs_with_address(kvm, gfn_start,
						   gfn_end - gfn_start);

	kvm_mmu_invalidate_end(kvm, 0, -1ul);

	write_unlock(&kvm->mmu_lock);
}

static bool slot_rmap_write_protect(struct kvm *kvm,
				    struct kvm_rmap_head *rmap_head,
				    const struct kvm_memory_slot *slot)
{
	return rmap_write_protect(rmap_head, false);
}

void kvm_mmu_slot_remove_write_access(struct kvm *kvm,
				      const struct kvm_memory_slot *memslot,
				      int start_level)
{
	if (kvm_memslots_have_rmaps(kvm)) {
		write_lock(&kvm->mmu_lock);
		slot_handle_level(kvm, memslot, slot_rmap_write_protect,
				  start_level, KVM_MAX_HUGEPAGE_LEVEL, false);
		write_unlock(&kvm->mmu_lock);
	}

	if (is_tdp_mmu_enabled(kvm)) {
		read_lock(&kvm->mmu_lock);
		kvm_tdp_mmu_wrprot_slot(kvm, memslot, start_level);
		read_unlock(&kvm->mmu_lock);
	}
}

static inline bool need_topup(struct kvm_mmu_memory_cache *cache, int min)
{
	return kvm_mmu_memory_cache_nr_free_objects(cache) < min;
}

static bool need_topup_split_caches_or_resched(struct kvm *kvm)
{
	if (need_resched() || rwlock_needbreak(&kvm->mmu_lock))
		return true;

	/*
	 * In the worst case, SPLIT_DESC_CACHE_MIN_NR_OBJECTS descriptors are needed
	 * to split a single huge page. Calculating how many are actually needed
	 * is possible but not worth the complexity.
	 */
	return need_topup(&kvm->arch.split_desc_cache, SPLIT_DESC_CACHE_MIN_NR_OBJECTS) ||
	       need_topup(&kvm->arch.split_page_header_cache, 1) ||
	       need_topup(&kvm->arch.split_shadow_page_cache, 1);
}

static int topup_split_caches(struct kvm *kvm)
{
	/*
	 * Allocating rmap list entries when splitting huge pages for nested
	 * MMUs is uncommon as KVM needs to use a list if and only if there is
	 * more than one rmap entry for a gfn, i.e. requires an L1 gfn to be
	 * aliased by multiple L2 gfns and/or from multiple nested roots with
	 * different roles.  Aliasing gfns when using TDP is atypical for VMMs;
	 * a few gfns are often aliased during boot, e.g. when remapping BIOS,
	 * but aliasing rarely occurs post-boot or for many gfns.  If there is
	 * only one rmap entry, rmap->val points directly at that one entry and
	 * doesn't need to allocate a list.  Buffer the cache by the default
	 * capacity so that KVM doesn't have to drop mmu_lock to topup if KVM
	 * encounters an aliased gfn or two.
	 */
	const int capacity = SPLIT_DESC_CACHE_MIN_NR_OBJECTS +
			     KVM_ARCH_NR_OBJS_PER_MEMORY_CACHE;
	int r;

	lockdep_assert_held(&kvm->slots_lock);

	r = __kvm_mmu_topup_memory_cache(&kvm->arch.split_desc_cache, capacity,
					 SPLIT_DESC_CACHE_MIN_NR_OBJECTS);
	if (r)
		return r;

	r = kvm_mmu_topup_memory_cache(&kvm->arch.split_page_header_cache, 1);
	if (r)
		return r;

	return kvm_mmu_topup_memory_cache(&kvm->arch.split_shadow_page_cache, 1);
}

static struct kvm_mmu_page *shadow_mmu_get_sp_for_split(struct kvm *kvm, u64 *huge_sptep)
{
	struct kvm_mmu_page *huge_sp = sptep_to_sp(huge_sptep);
	struct shadow_page_caches caches = {};
	union kvm_mmu_page_role role;
	unsigned int access;
	gfn_t gfn;

	gfn = kvm_mmu_page_get_gfn(huge_sp, spte_index(huge_sptep));
	access = kvm_mmu_page_get_access(huge_sp, spte_index(huge_sptep));

	/*
	 * Note, huge page splitting always uses direct shadow pages, regardless
	 * of whether the huge page itself is mapped by a direct or indirect
	 * shadow page, since the huge page region itself is being directly
	 * mapped with smaller pages.
	 */
	role = kvm_mmu_child_role(huge_sptep, /*direct=*/true, access);

	/* Direct SPs do not require a shadowed_info_cache. */
	caches.page_header_cache = &kvm->arch.split_page_header_cache;
	caches.shadow_page_cache = &kvm->arch.split_shadow_page_cache;

	/* Safe to pass NULL for vCPU since requesting a direct SP. */
	return __kvm_mmu_get_shadow_page(kvm, NULL, &caches, gfn, role);
}

static void shadow_mmu_split_huge_page(struct kvm *kvm,
				       const struct kvm_memory_slot *slot,
				       u64 *huge_sptep)

{
	struct kvm_mmu_memory_cache *cache = &kvm->arch.split_desc_cache;
	u64 huge_spte = READ_ONCE(*huge_sptep);
	struct kvm_mmu_page *sp;
	bool flush = false;
	u64 *sptep, spte;
	gfn_t gfn;
	int index;

	sp = shadow_mmu_get_sp_for_split(kvm, huge_sptep);

	for (index = 0; index < SPTE_ENT_PER_PAGE; index++) {
		sptep = &sp->spt[index];
		gfn = kvm_mmu_page_get_gfn(sp, index);

		/*
		 * The SP may already have populated SPTEs, e.g. if this huge
		 * page is aliased by multiple sptes with the same access
		 * permissions. These entries are guaranteed to map the same
		 * gfn-to-pfn translation since the SP is direct, so no need to
		 * modify them.
		 *
		 * However, if a given SPTE points to a lower level page table,
		 * that lower level page table may only be partially populated.
		 * Installing such SPTEs would effectively unmap a potion of the
		 * huge page. Unmapping guest memory always requires a TLB flush
		 * since a subsequent operation on the unmapped regions would
		 * fail to detect the need to flush.
		 */
		if (is_shadow_present_pte(*sptep)) {
			flush |= !is_last_spte(*sptep, sp->role.level);
			continue;
		}

		spte = make_huge_page_split_spte(kvm, huge_spte, sp->role, index);
		mmu_spte_set(sptep, spte);
		__rmap_add(kvm, cache, slot, sptep, gfn, sp->role.access);
	}

	__link_shadow_page(kvm, cache, huge_sptep, sp, flush);
}

static int shadow_mmu_try_split_huge_page(struct kvm *kvm,
					  const struct kvm_memory_slot *slot,
					  u64 *huge_sptep)
{
	struct kvm_mmu_page *huge_sp = sptep_to_sp(huge_sptep);
	int level, r = 0;
	gfn_t gfn;
	u64 spte;

	/* Grab information for the tracepoint before dropping the MMU lock. */
	gfn = kvm_mmu_page_get_gfn(huge_sp, spte_index(huge_sptep));
	level = huge_sp->role.level;
	spte = *huge_sptep;

	if (kvm_mmu_available_pages(kvm) <= KVM_MIN_FREE_MMU_PAGES) {
		r = -ENOSPC;
		goto out;
	}

	if (need_topup_split_caches_or_resched(kvm)) {
		write_unlock(&kvm->mmu_lock);
		cond_resched();
		/*
		 * If the topup succeeds, return -EAGAIN to indicate that the
		 * rmap iterator should be restarted because the MMU lock was
		 * dropped.
		 */
		r = topup_split_caches(kvm) ?: -EAGAIN;
		write_lock(&kvm->mmu_lock);
		goto out;
	}

	shadow_mmu_split_huge_page(kvm, slot, huge_sptep);

out:
	trace_kvm_mmu_split_huge_page(gfn, spte, level, r);
	return r;
}

static bool shadow_mmu_try_split_huge_pages(struct kvm *kvm,
					    struct kvm_rmap_head *rmap_head,
					    const struct kvm_memory_slot *slot)
{
	struct rmap_iterator iter;
	struct kvm_mmu_page *sp;
	u64 *huge_sptep;
	int r;

restart:
	for_each_rmap_spte(rmap_head, &iter, huge_sptep) {
		sp = sptep_to_sp(huge_sptep);

		/* TDP MMU is enabled, so rmap only contains nested MMU SPs. */
		if (WARN_ON_ONCE(!sp->role.guest_mode))
			continue;

		/* The rmaps should never contain non-leaf SPTEs. */
		if (WARN_ON_ONCE(!is_large_pte(*huge_sptep)))
			continue;

		/* SPs with level >PG_LEVEL_4K should never by unsync. */
		if (WARN_ON_ONCE(sp->unsync))
			continue;

		/* Don't bother splitting huge pages on invalid SPs. */
		if (sp->role.invalid)
			continue;

		r = shadow_mmu_try_split_huge_page(kvm, slot, huge_sptep);

		/*
		 * The split succeeded or needs to be retried because the MMU
		 * lock was dropped. Either way, restart the iterator to get it
		 * back into a consistent state.
		 */
		if (!r || r == -EAGAIN)
			goto restart;

		/* The split failed and shouldn't be retried (e.g. -ENOMEM). */
		break;
	}

	return false;
}

static void kvm_shadow_mmu_try_split_huge_pages(struct kvm *kvm,
						const struct kvm_memory_slot *slot,
						gfn_t start, gfn_t end,
						int target_level)
{
	int level;

	/*
	 * Split huge pages starting with KVM_MAX_HUGEPAGE_LEVEL and working
	 * down to the target level. This ensures pages are recursively split
	 * all the way to the target level. There's no need to split pages
	 * already at the target level.
	 */
	for (level = KVM_MAX_HUGEPAGE_LEVEL; level > target_level; level--) {
		slot_handle_level_range(kvm, slot, shadow_mmu_try_split_huge_pages,
					level, level, start, end - 1, true, false);
	}
}

/* Must be called with the mmu_lock held in write-mode. */
void kvm_mmu_try_split_huge_pages(struct kvm *kvm,
				   const struct kvm_memory_slot *memslot,
				   u64 start, u64 end,
				   int target_level)
{
	if (!is_tdp_mmu_enabled(kvm))
		return;

	if (kvm_memslots_have_rmaps(kvm))
		kvm_shadow_mmu_try_split_huge_pages(kvm, memslot, start, end, target_level);

	kvm_tdp_mmu_try_split_huge_pages(kvm, memslot, start, end, target_level, false);

	/*
	 * A TLB flush is unnecessary at this point for the same resons as in
	 * kvm_mmu_slot_try_split_huge_pages().
	 */
}

void kvm_mmu_slot_try_split_huge_pages(struct kvm *kvm,
					const struct kvm_memory_slot *memslot,
					int target_level)
{
	u64 start = memslot->base_gfn;
	u64 end = start + memslot->npages;

	if (!is_tdp_mmu_enabled(kvm))
		return;

	if (kvm_memslots_have_rmaps(kvm)) {
		write_lock(&kvm->mmu_lock);
		kvm_shadow_mmu_try_split_huge_pages(kvm, memslot, start, end, target_level);
		write_unlock(&kvm->mmu_lock);
	}

	read_lock(&kvm->mmu_lock);
	kvm_tdp_mmu_try_split_huge_pages(kvm, memslot, start, end, target_level, true);
	read_unlock(&kvm->mmu_lock);

	/*
	 * No TLB flush is necessary here. KVM will flush TLBs after
	 * write-protecting and/or clearing dirty on the newly split SPTEs to
	 * ensure that guest writes are reflected in the dirty log before the
	 * ioctl to enable dirty logging on this memslot completes. Since the
	 * split SPTEs retain the write and dirty bits of the huge SPTE, it is
	 * safe for KVM to decide if a TLB flush is necessary based on the split
	 * SPTEs.
	 */
}

static bool kvm_mmu_zap_collapsible_spte(struct kvm *kvm,
					 struct kvm_rmap_head *rmap_head,
					 const struct kvm_memory_slot *slot)
{
	u64 *sptep;
	struct rmap_iterator iter;
	int need_tlb_flush = 0;
	struct kvm_mmu_page *sp;

restart:
	for_each_rmap_spte(rmap_head, &iter, sptep) {
		sp = sptep_to_sp(sptep);

		/*
		 * We cannot do huge page mapping for indirect shadow pages,
		 * which are found on the last rmap (level = 1) when not using
		 * tdp; such shadow pages are synced with the page table in
		 * the guest, and the guest page table is using 4K page size
		 * mapping if the indirect sp has level = 1.
		 */
		if (sp->role.direct &&
		    sp->role.level < kvm_mmu_max_mapping_level(kvm, slot, sp->gfn,
							       PG_LEVEL_NUM)) {
			kvm_zap_one_rmap_spte(kvm, rmap_head, sptep);

			if (kvm_available_flush_tlb_with_range())
				kvm_flush_remote_tlbs_with_address(kvm, sp->gfn,
					KVM_PAGES_PER_HPAGE(sp->role.level));
			else
				need_tlb_flush = 1;

			goto restart;
		}
	}

	return need_tlb_flush;
}

static void kvm_rmap_zap_collapsible_sptes(struct kvm *kvm,
					   const struct kvm_memory_slot *slot)
{
	/*
	 * Note, use KVM_MAX_HUGEPAGE_LEVEL - 1 since there's no need to zap
	 * pages that are already mapped at the maximum hugepage level.
	 */
	if (slot_handle_level(kvm, slot, kvm_mmu_zap_collapsible_spte,
			      PG_LEVEL_4K, KVM_MAX_HUGEPAGE_LEVEL - 1, true))
		kvm_arch_flush_remote_tlbs_memslot(kvm, slot);
}

void kvm_mmu_zap_collapsible_sptes(struct kvm *kvm,
				   const struct kvm_memory_slot *slot)
{
	if (kvm_memslots_have_rmaps(kvm)) {
		write_lock(&kvm->mmu_lock);
		kvm_rmap_zap_collapsible_sptes(kvm, slot);
		write_unlock(&kvm->mmu_lock);
	}

	if (is_tdp_mmu_enabled(kvm)) {
		read_lock(&kvm->mmu_lock);
		kvm_tdp_mmu_zap_collapsible_sptes(kvm, slot);
		read_unlock(&kvm->mmu_lock);
	}
}

void kvm_arch_flush_remote_tlbs_memslot(struct kvm *kvm,
					const struct kvm_memory_slot *memslot)
{
	/*
	 * All current use cases for flushing the TLBs for a specific memslot
	 * related to dirty logging, and many do the TLB flush out of mmu_lock.
	 * The interaction between the various operations on memslot must be
	 * serialized by slots_locks to ensure the TLB flush from one operation
	 * is observed by any other operation on the same memslot.
	 */
	lockdep_assert_held(&kvm->slots_lock);
	kvm_flush_remote_tlbs_with_address(kvm, memslot->base_gfn,
					   memslot->npages);
}

void kvm_mmu_slot_leaf_clear_dirty(struct kvm *kvm,
				   const struct kvm_memory_slot *memslot)
{
	if (kvm_memslots_have_rmaps(kvm)) {
		write_lock(&kvm->mmu_lock);
		/*
		 * Clear dirty bits only on 4k SPTEs since the legacy MMU only
		 * support dirty logging at a 4k granularity.
		 */
		slot_handle_level_4k(kvm, memslot, __rmap_clear_dirty, false);
		write_unlock(&kvm->mmu_lock);
	}

	if (is_tdp_mmu_enabled(kvm)) {
		read_lock(&kvm->mmu_lock);
		kvm_tdp_mmu_clear_dirty_slot(kvm, memslot);
		read_unlock(&kvm->mmu_lock);
	}

	/*
	 * The caller will flush the TLBs after this function returns.
	 *
	 * It's also safe to flush TLBs out of mmu lock here as currently this
	 * function is only used for dirty logging, in which case flushing TLB
	 * out of mmu lock also guarantees no dirty pages will be lost in
	 * dirty_bitmap.
	 */
}

void kvm_mmu_zap_all(struct kvm *kvm)
{
	struct kvm_mmu_page *sp, *node;
	LIST_HEAD(invalid_list);
	int ign;

	write_lock(&kvm->mmu_lock);
restart:
	list_for_each_entry_safe(sp, node, &kvm->arch.active_mmu_pages, link) {
		if (WARN_ON(sp->role.invalid))
			continue;
		if (__kvm_mmu_prepare_zap_page(kvm, sp, &invalid_list, &ign))
			goto restart;
		if (cond_resched_rwlock_write(&kvm->mmu_lock))
			goto restart;
	}

	kvm_mmu_commit_zap_page(kvm, &invalid_list);

	if (is_tdp_mmu_enabled(kvm))
		kvm_tdp_mmu_zap_all(kvm);

	write_unlock(&kvm->mmu_lock);
}

void kvm_mmu_invalidate_mmio_sptes(struct kvm *kvm, u64 gen)
{
	WARN_ON(gen & KVM_MEMSLOT_GEN_UPDATE_IN_PROGRESS);

	gen &= MMIO_SPTE_GEN_MASK;

	/*
	 * Generation numbers are incremented in multiples of the number of
	 * address spaces in order to provide unique generations across all
	 * address spaces.  Strip what is effectively the address space
	 * modifier prior to checking for a wrap of the MMIO generation so
	 * that a wrap in any address space is detected.
	 */
	gen &= ~((u64)KVM_ADDRESS_SPACE_NUM - 1);

	/*
	 * The very rare case: if the MMIO generation number has wrapped,
	 * zap all shadow pages.
	 */
	if (unlikely(gen == 0)) {
		kvm_debug_ratelimited("kvm: zapping shadow pages for mmio generation wraparound\n");
		kvm_mmu_zap_all_fast(kvm);
	}
}

static unsigned long
mmu_shrink_scan(struct shrinker *shrink, struct shrink_control *sc)
{
	struct kvm *kvm;
	int nr_to_scan = sc->nr_to_scan;
	unsigned long freed = 0;

	mutex_lock(&kvm_lock);

	list_for_each_entry(kvm, &vm_list, vm_list) {
		int idx;
		LIST_HEAD(invalid_list);

		/*
		 * Never scan more than sc->nr_to_scan VM instances.
		 * Will not hit this condition practically since we do not try
		 * to shrink more than one VM and it is very unlikely to see
		 * !n_used_mmu_pages so many times.
		 */
		if (!nr_to_scan--)
			break;
		/*
		 * n_used_mmu_pages is accessed without holding kvm->mmu_lock
		 * here. We may skip a VM instance errorneosly, but we do not
		 * want to shrink a VM that only started to populate its MMU
		 * anyway.
		 */
		if (!kvm->arch.n_used_mmu_pages &&
		    !kvm_has_zapped_obsolete_pages(kvm))
			continue;

		idx = srcu_read_lock(&kvm->srcu);
		write_lock(&kvm->mmu_lock);

		if (kvm_has_zapped_obsolete_pages(kvm)) {
			kvm_mmu_commit_zap_page(kvm,
			      &kvm->arch.zapped_obsolete_pages);
			goto unlock;
		}

		freed = kvm_mmu_zap_oldest_mmu_pages(kvm, sc->nr_to_scan);

unlock:
		write_unlock(&kvm->mmu_lock);
		srcu_read_unlock(&kvm->srcu, idx);

		/*
		 * unfair on small ones
		 * per-vm shrinkers cry out
		 * sadness comes quickly
		 */
		list_move_tail(&kvm->vm_list, &vm_list);
		break;
	}

	mutex_unlock(&kvm_lock);
	return freed;
}

static unsigned long
mmu_shrink_count(struct shrinker *shrink, struct shrink_control *sc)
{
	return percpu_counter_read_positive(&kvm_total_used_mmu_pages);
}

static struct shrinker mmu_shrinker = {
	.count_objects = mmu_shrink_count,
	.scan_objects = mmu_shrink_scan,
	.seeks = DEFAULT_SEEKS * 10,
};

static void mmu_destroy_caches(void)
{
	kmem_cache_destroy(pte_list_desc_cache);
	kmem_cache_destroy(mmu_page_header_cache);
}

static bool get_nx_auto_mode(void)
{
	/* Return true when CPU has the bug, and mitigations are ON */
	return boot_cpu_has_bug(X86_BUG_ITLB_MULTIHIT) && !cpu_mitigations_off();
}

static void __set_nx_huge_pages(bool val)
{
	nx_huge_pages = itlb_multihit_kvm_mitigation = val;
}

static int set_nx_huge_pages(const char *val, const struct kernel_param *kp)
{
	bool old_val = nx_huge_pages;
	bool new_val;

	/* In "auto" mode deploy workaround only if CPU has the bug. */
	if (sysfs_streq(val, "off"))
		new_val = 0;
	else if (sysfs_streq(val, "force"))
		new_val = 1;
	else if (sysfs_streq(val, "auto"))
		new_val = get_nx_auto_mode();
	else if (strtobool(val, &new_val) < 0)
		return -EINVAL;

	__set_nx_huge_pages(new_val);

	if (new_val != old_val) {
		struct kvm *kvm;

		mutex_lock(&kvm_lock);

		list_for_each_entry(kvm, &vm_list, vm_list) {
			mutex_lock(&kvm->slots_lock);
			kvm_mmu_zap_all_fast(kvm);
			mutex_unlock(&kvm->slots_lock);

			wake_up_process(kvm->arch.nx_lpage_recovery_thread);
		}
		mutex_unlock(&kvm_lock);
	}

	return 0;
}

/*
 * nx_huge_pages needs to be resolved to true/false when kvm.ko is loaded, as
 * its default value of -1 is technically undefined behavior for a boolean.
 * Forward the module init call to SPTE code so that it too can handle module
 * params that need to be resolved/snapshot.
 */
void __init kvm_mmu_x86_module_init(void)
{
	if (nx_huge_pages == -1)
		__set_nx_huge_pages(get_nx_auto_mode());

	kvm_mmu_spte_module_init();
}

/*
 * The bulk of the MMU initialization is deferred until the vendor module is
 * loaded as many of the masks/values may be modified by VMX or SVM, i.e. need
 * to be reset when a potentially different vendor module is loaded.
 */
int kvm_mmu_vendor_module_init(void)
{
	int ret = -ENOMEM;

	/*
	 * MMU roles use union aliasing which is, generally speaking, an
	 * undefined behavior. However, we supposedly know how compilers behave
	 * and the current status quo is unlikely to change. Guardians below are
	 * supposed to let us know if the assumption becomes false.
	 */
	BUILD_BUG_ON(sizeof(union kvm_mmu_page_role) != sizeof(u32));
	BUILD_BUG_ON(sizeof(union kvm_mmu_extended_role) != sizeof(u32));
	BUILD_BUG_ON(sizeof(union kvm_cpu_role) != sizeof(u64));

	kvm_mmu_reset_all_pte_masks();

	pte_list_desc_cache = kmem_cache_create("pte_list_desc",
					    sizeof(struct pte_list_desc),
					    0, SLAB_ACCOUNT, NULL);
	if (!pte_list_desc_cache)
		goto out;

	mmu_page_header_cache = kmem_cache_create("kvm_mmu_page_header",
						  sizeof(struct kvm_mmu_page),
						  0, SLAB_ACCOUNT, NULL);
	if (!mmu_page_header_cache)
		goto out;

	if (percpu_counter_init(&kvm_total_used_mmu_pages, 0, GFP_KERNEL))
		goto out;

	ret = register_shrinker(&mmu_shrinker, "x86-mmu");
	if (ret)
		goto out_shrinker;

	return 0;

out_shrinker:
	percpu_counter_destroy(&kvm_total_used_mmu_pages);
out:
	mmu_destroy_caches();
	return ret;
}

void kvm_mmu_destroy(struct kvm_vcpu *vcpu)
{
	kvm_mmu_unload(vcpu);
	free_mmu_pages(&vcpu->arch.root_mmu);
	free_mmu_pages(&vcpu->arch.guest_mmu);
	mmu_free_memory_caches(vcpu);
}

void kvm_mmu_vendor_module_exit(void)
{
	mmu_destroy_caches();
	percpu_counter_destroy(&kvm_total_used_mmu_pages);
	unregister_shrinker(&mmu_shrinker);
}

/*
 * Calculate the effective recovery period, accounting for '0' meaning "let KVM
 * select a halving time of 1 hour".  Returns true if recovery is enabled.
 */
static bool calc_nx_huge_pages_recovery_period(uint *period)
{
	/*
	 * Use READ_ONCE to get the params, this may be called outside of the
	 * param setters, e.g. by the kthread to compute its next timeout.
	 */
	bool enabled = READ_ONCE(nx_huge_pages);
	uint ratio = READ_ONCE(nx_huge_pages_recovery_ratio);

	if (!enabled || !ratio)
		return false;

	*period = READ_ONCE(nx_huge_pages_recovery_period_ms);
	if (!*period) {
		/* Make sure the period is not less than one second.  */
		ratio = min(ratio, 3600u);
		*period = 60 * 60 * 1000 / ratio;
	}
	return true;
}

static int set_nx_huge_pages_recovery_param(const char *val, const struct kernel_param *kp)
{
	bool was_recovery_enabled, is_recovery_enabled;
	uint old_period, new_period;
	int err;

	was_recovery_enabled = calc_nx_huge_pages_recovery_period(&old_period);

	err = param_set_uint(val, kp);
	if (err)
		return err;

	is_recovery_enabled = calc_nx_huge_pages_recovery_period(&new_period);

	if (is_recovery_enabled &&
	    (!was_recovery_enabled || old_period > new_period)) {
		struct kvm *kvm;

		mutex_lock(&kvm_lock);

		list_for_each_entry(kvm, &vm_list, vm_list)
			wake_up_process(kvm->arch.nx_lpage_recovery_thread);

		mutex_unlock(&kvm_lock);
	}

	return err;
}

static void kvm_recover_nx_lpages(struct kvm *kvm)
{
	unsigned long nx_lpage_splits = kvm->stat.nx_lpage_splits;
	int rcu_idx;
	struct kvm_mmu_page *sp;
	unsigned int ratio;
	LIST_HEAD(invalid_list);
	bool flush = false;
	ulong to_zap;

	rcu_idx = srcu_read_lock(&kvm->srcu);
	write_lock(&kvm->mmu_lock);

	/*
	 * Zapping TDP MMU shadow pages, including the remote TLB flush, must
	 * be done under RCU protection, because the pages are freed via RCU
	 * callback.
	 */
	rcu_read_lock();

	ratio = READ_ONCE(nx_huge_pages_recovery_ratio);
	to_zap = ratio ? DIV_ROUND_UP(nx_lpage_splits, ratio) : 0;
	for ( ; to_zap; --to_zap) {
		if (list_empty(&kvm->arch.lpage_disallowed_mmu_pages))
			break;

		/*
		 * We use a separate list instead of just using active_mmu_pages
		 * because the number of lpage_disallowed pages is expected to
		 * be relatively small compared to the total.
		 */
		sp = list_first_entry(&kvm->arch.lpage_disallowed_mmu_pages,
				      struct kvm_mmu_page,
				      lpage_disallowed_link);
		WARN_ON_ONCE(!sp->lpage_disallowed);
		if (is_tdp_mmu_page(sp)) {
			flush |= kvm_tdp_mmu_zap_sp(kvm, sp);
		} else {
			kvm_mmu_prepare_zap_page(kvm, sp, &invalid_list);
			WARN_ON_ONCE(sp->lpage_disallowed);
		}

		if (need_resched() || rwlock_needbreak(&kvm->mmu_lock)) {
			kvm_mmu_remote_flush_or_zap(kvm, &invalid_list, flush);
			rcu_read_unlock();

			cond_resched_rwlock_write(&kvm->mmu_lock);
			flush = false;

			rcu_read_lock();
		}
	}
	kvm_mmu_remote_flush_or_zap(kvm, &invalid_list, flush);

	rcu_read_unlock();

	write_unlock(&kvm->mmu_lock);
	srcu_read_unlock(&kvm->srcu, rcu_idx);
}

static long get_nx_lpage_recovery_timeout(u64 start_time)
{
	bool enabled;
	uint period;

	enabled = calc_nx_huge_pages_recovery_period(&period);

	return enabled ? start_time + msecs_to_jiffies(period) - get_jiffies_64()
		       : MAX_SCHEDULE_TIMEOUT;
}

static int kvm_nx_lpage_recovery_worker(struct kvm *kvm, uintptr_t data)
{
	u64 start_time;
	long remaining_time;

	while (true) {
		start_time = get_jiffies_64();
		remaining_time = get_nx_lpage_recovery_timeout(start_time);

		set_current_state(TASK_INTERRUPTIBLE);
		while (!kthread_should_stop() && remaining_time > 0) {
			schedule_timeout(remaining_time);
			remaining_time = get_nx_lpage_recovery_timeout(start_time);
			set_current_state(TASK_INTERRUPTIBLE);
		}

		set_current_state(TASK_RUNNING);

		if (kthread_should_stop())
			return 0;

		kvm_recover_nx_lpages(kvm);
	}
}

int kvm_mmu_post_init_vm(struct kvm *kvm)
{
	int err;

	err = kvm_vm_create_worker_thread(kvm, kvm_nx_lpage_recovery_worker, 0,
					  "kvm-nx-lpage-recovery",
					  &kvm->arch.nx_lpage_recovery_thread);
	if (!err)
		kthread_unpark(kvm->arch.nx_lpage_recovery_thread);

	return err;
}

void kvm_mmu_pre_destroy_vm(struct kvm *kvm)
{
	if (kvm->arch.nx_lpage_recovery_thread)
		kthread_stop(kvm->arch.nx_lpage_recovery_thread);
}