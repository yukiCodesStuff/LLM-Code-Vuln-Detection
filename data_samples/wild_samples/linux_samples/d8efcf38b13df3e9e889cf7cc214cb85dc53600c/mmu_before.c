		if (iterator.level == level) {
			mmu_set_spte(vcpu, iterator.sptep, ACC_ALL,
				     write, &emulate, level, gfn, pfn,
				     prefault, map_writable);
			direct_pte_prefetch(vcpu, iterator.sptep);
			++vcpu->stat.pf_fixed;
			break;
		}

		if (!is_shadow_present_pte(*iterator.sptep)) {
			u64 base_addr = iterator.addr;

			base_addr &= PT64_LVL_ADDR_MASK(iterator.level);
			pseudo_gfn = base_addr >> PAGE_SHIFT;
			sp = kvm_mmu_get_page(vcpu, pseudo_gfn, iterator.addr,
					      iterator.level - 1,
					      1, ACC_ALL, iterator.sptep);

			link_shadow_page(iterator.sptep, sp, true);
		}
	}
	return emulate;
}

static void kvm_send_hwpoison_signal(unsigned long address, struct task_struct *tsk)
{
	siginfo_t info;

	info.si_signo	= SIGBUS;
	info.si_errno	= 0;
	info.si_code	= BUS_MCEERR_AR;
	info.si_addr	= (void __user *)address;
	info.si_addr_lsb = PAGE_SHIFT;

	send_sig_info(SIGBUS, &info, tsk);
}

static int kvm_handle_bad_page(struct kvm_vcpu *vcpu, gfn_t gfn, pfn_t pfn)
{
	/*
	 * Do not cache the mmio info caused by writing the readonly gfn
	 * into the spte otherwise read access on readonly gfn also can
	 * caused mmio page fault and treat it as mmio access.
	 * Return 1 to tell kvm to emulate it.
	 */
	if (pfn == KVM_PFN_ERR_RO_FAULT)
		return 1;

	if (pfn == KVM_PFN_ERR_HWPOISON) {
		kvm_send_hwpoison_signal(gfn_to_hva(vcpu->kvm, gfn), current);
		return 0;
	}

	return -EFAULT;
}

static void transparent_hugepage_adjust(struct kvm_vcpu *vcpu,
					gfn_t *gfnp, pfn_t *pfnp, int *levelp)
{
	pfn_t pfn = *pfnp;
	gfn_t gfn = *gfnp;
	int level = *levelp;

	/*
	 * Check if it's a transparent hugepage. If this would be an
	 * hugetlbfs page, level wouldn't be set to
	 * PT_PAGE_TABLE_LEVEL and there would be no adjustment done
	 * here.
	 */
	if (!is_error_noslot_pfn(pfn) && !kvm_is_mmio_pfn(pfn) &&
	    level == PT_PAGE_TABLE_LEVEL &&
	    PageTransCompound(pfn_to_page(pfn)) &&
	    !has_wrprotected_page(vcpu->kvm, gfn, PT_DIRECTORY_LEVEL)) {
		unsigned long mask;
		/*
		 * mmu_notifier_retry was successful and we hold the
		 * mmu_lock here, so the pmd can't become splitting
		 * from under us, and in turn
		 * __split_huge_page_refcount() can't run from under
		 * us and we can safely transfer the refcount from
		 * PG_tail to PG_head as we switch the pfn to tail to
		 * head.
		 */
		*levelp = level = PT_DIRECTORY_LEVEL;
		mask = KVM_PAGES_PER_HPAGE(level) - 1;
		VM_BUG_ON((gfn & mask) != (pfn & mask));
		if (pfn & mask) {
			gfn &= ~mask;
			*gfnp = gfn;
			kvm_release_pfn_clean(pfn);
			pfn &= ~mask;
			kvm_get_pfn(pfn);
			*pfnp = pfn;
		}
	}
}

static bool handle_abnormal_pfn(struct kvm_vcpu *vcpu, gva_t gva, gfn_t gfn,
				pfn_t pfn, unsigned access, int *ret_val)
{
	bool ret = true;

	/* The pfn is invalid, report the error! */
	if (unlikely(is_error_pfn(pfn))) {
		*ret_val = kvm_handle_bad_page(vcpu, gfn, pfn);
		goto exit;
	}

	if (unlikely(is_noslot_pfn(pfn)))
		vcpu_cache_mmio_info(vcpu, gva, gfn, access);

	ret = false;
exit:
	return ret;
}

static bool page_fault_can_be_fast(u32 error_code)
{
	/*
	 * Do not fix the mmio spte with invalid generation number which
	 * need to be updated by slow page fault path.
	 */
	if (unlikely(error_code & PFERR_RSVD_MASK))
		return false;

	/*
	 * #PF can be fast only if the shadow page table is present and it
	 * is caused by write-protect, that means we just need change the
	 * W bit of the spte which can be done out of mmu-lock.
	 */
	if (!(error_code & PFERR_PRESENT_MASK) ||
	      !(error_code & PFERR_WRITE_MASK))
		return false;

	return true;
}

static bool
fast_pf_fix_direct_spte(struct kvm_vcpu *vcpu, u64 *sptep, u64 spte)
{
	struct kvm_mmu_page *sp = page_header(__pa(sptep));
	gfn_t gfn;

	WARN_ON(!sp->role.direct);

	/*
	 * The gfn of direct spte is stable since it is calculated
	 * by sp->gfn.
	 */
	gfn = kvm_mmu_page_get_gfn(sp, sptep - sp->spt);

	if (cmpxchg64(sptep, spte, spte | PT_WRITABLE_MASK) == spte)
		mark_page_dirty(vcpu->kvm, gfn);

	return true;
}

/*
 * Return value:
 * - true: let the vcpu to access on the same address again.
 * - false: let the real page fault path to fix it.
 */
static bool fast_page_fault(struct kvm_vcpu *vcpu, gva_t gva, int level,
			    u32 error_code)
{
	struct kvm_shadow_walk_iterator iterator;
	bool ret = false;
	u64 spte = 0ull;

	if (!VALID_PAGE(vcpu->arch.mmu.root_hpa))
		return false;

	if (!page_fault_can_be_fast(error_code))
		return false;

	walk_shadow_page_lockless_begin(vcpu);
	for_each_shadow_entry_lockless(vcpu, gva, iterator, spte)
		if (!is_shadow_present_pte(spte) || iterator.level < level)
			break;

	/*
	 * If the mapping has been changed, let the vcpu fault on the
	 * same address again.
	 */
	if (!is_rmap_spte(spte)) {
		ret = true;
		goto exit;
	}

	if (!is_last_spte(spte, level))
		goto exit;

	/*
	 * Check if it is a spurious fault caused by TLB lazily flushed.
	 *
	 * Need not check the access of upper level table entries since
	 * they are always ACC_ALL.
	 */
	 if (is_writable_pte(spte)) {
		ret = true;
		goto exit;
	}

	/*
	 * Currently, to simplify the code, only the spte write-protected
	 * by dirty-log can be fast fixed.
	 */
	if (!spte_is_locklessly_modifiable(spte))
		goto exit;

	/*
	 * Currently, fast page fault only works for direct mapping since
	 * the gfn is not stable for indirect shadow page.
	 * See Documentation/virtual/kvm/locking.txt to get more detail.
	 */
	ret = fast_pf_fix_direct_spte(vcpu, iterator.sptep, spte);
exit:
	trace_fast_page_fault(vcpu, gva, error_code, iterator.sptep,
			      spte, ret);
	walk_shadow_page_lockless_end(vcpu);

	return ret;
}

static bool try_async_pf(struct kvm_vcpu *vcpu, bool prefault, gfn_t gfn,
			 gva_t gva, pfn_t *pfn, bool write, bool *writable);
static void make_mmu_pages_available(struct kvm_vcpu *vcpu);

static int nonpaging_map(struct kvm_vcpu *vcpu, gva_t v, u32 error_code,
			 gfn_t gfn, bool prefault)
{
	int r;
	int level;
	int force_pt_level;
	pfn_t pfn;
	unsigned long mmu_seq;
	bool map_writable, write = error_code & PFERR_WRITE_MASK;

	force_pt_level = mapping_level_dirty_bitmap(vcpu, gfn);
	if (likely(!force_pt_level)) {
		level = mapping_level(vcpu, gfn);
		/*
		 * This path builds a PAE pagetable - so we can map
		 * 2mb pages at maximum. Therefore check if the level
		 * is larger than that.
		 */
		if (level > PT_DIRECTORY_LEVEL)
			level = PT_DIRECTORY_LEVEL;

		gfn &= ~(KVM_PAGES_PER_HPAGE(level) - 1);
	} else
		level = PT_PAGE_TABLE_LEVEL;

	if (fast_page_fault(vcpu, v, level, error_code))
		return 0;

	mmu_seq = vcpu->kvm->mmu_notifier_seq;
	smp_rmb();

	if (try_async_pf(vcpu, prefault, gfn, v, &pfn, write, &map_writable))
		return 0;

	if (handle_abnormal_pfn(vcpu, v, gfn, pfn, ACC_ALL, &r))
		return r;

	spin_lock(&vcpu->kvm->mmu_lock);
	if (mmu_notifier_retry(vcpu->kvm, mmu_seq))
		goto out_unlock;
	make_mmu_pages_available(vcpu);
	if (likely(!force_pt_level))
		transparent_hugepage_adjust(vcpu, &gfn, &pfn, &level);
	r = __direct_map(vcpu, v, write, map_writable, level, gfn, pfn,
			 prefault);
	spin_unlock(&vcpu->kvm->mmu_lock);


	return r;

out_unlock:
	spin_unlock(&vcpu->kvm->mmu_lock);
	kvm_release_pfn_clean(pfn);
	return 0;
}


static void mmu_free_roots(struct kvm_vcpu *vcpu)
{
	int i;
	struct kvm_mmu_page *sp;
	LIST_HEAD(invalid_list);

	if (!VALID_PAGE(vcpu->arch.mmu.root_hpa))
		return;

	if (vcpu->arch.mmu.shadow_root_level == PT64_ROOT_LEVEL &&
	    (vcpu->arch.mmu.root_level == PT64_ROOT_LEVEL ||
	     vcpu->arch.mmu.direct_map)) {
		hpa_t root = vcpu->arch.mmu.root_hpa;

		spin_lock(&vcpu->kvm->mmu_lock);
		sp = page_header(root);
		--sp->root_count;
		if (!sp->root_count && sp->role.invalid) {
			kvm_mmu_prepare_zap_page(vcpu->kvm, sp, &invalid_list);
			kvm_mmu_commit_zap_page(vcpu->kvm, &invalid_list);
		}