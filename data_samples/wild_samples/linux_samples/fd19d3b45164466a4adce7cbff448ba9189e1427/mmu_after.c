{
	struct kvm_mmu *context = &vcpu->arch.mmu;

	MMU_WARN_ON(VALID_PAGE(context->root_hpa));

	context->shadow_root_level = PT64_ROOT_4LEVEL;

	context->nx = true;
	context->ept_ad = accessed_dirty;
	context->page_fault = ept_page_fault;
	context->gva_to_gpa = ept_gva_to_gpa;
	context->sync_page = ept_sync_page;
	context->invlpg = ept_invlpg;
	context->update_pte = ept_update_pte;
	context->root_level = PT64_ROOT_4LEVEL;
	context->root_hpa = INVALID_PAGE;
	context->direct_map = false;
	context->base_role.ad_disabled = !accessed_dirty;

	update_permission_bitmask(vcpu, context, true);
	update_pkru_bitmask(vcpu, context, true);
	update_last_nonleaf_level(vcpu, context);
	reset_rsvds_bits_mask_ept(vcpu, context, execonly);
	reset_ept_shadow_zero_bits_mask(vcpu, context, execonly);
}
EXPORT_SYMBOL_GPL(kvm_init_shadow_ept_mmu);

static void init_kvm_softmmu(struct kvm_vcpu *vcpu)
{
	struct kvm_mmu *context = &vcpu->arch.mmu;

	kvm_init_shadow_mmu(vcpu);
	context->set_cr3           = kvm_x86_ops->set_cr3;
	context->get_cr3           = get_cr3;
	context->get_pdptr         = kvm_pdptr_read;
	context->inject_page_fault = kvm_inject_page_fault;
}

static void init_kvm_nested_mmu(struct kvm_vcpu *vcpu)
{
	struct kvm_mmu *g_context = &vcpu->arch.nested_mmu;

	g_context->get_cr3           = get_cr3;
	g_context->get_pdptr         = kvm_pdptr_read;
	g_context->inject_page_fault = kvm_inject_page_fault;

	/*
	 * Note that arch.mmu.gva_to_gpa translates l2_gpa to l1_gpa using
	 * L1's nested page tables (e.g. EPT12). The nested translation
	 * of l2_gva to l1_gpa is done by arch.nested_mmu.gva_to_gpa using
	 * L2's page tables as the first level of translation and L1's
	 * nested page tables as the second level of translation. Basically
	 * the gva_to_gpa functions between mmu and nested_mmu are swapped.
	 */
	if (!is_paging(vcpu)) {
		g_context->nx = false;
		g_context->root_level = 0;
		g_context->gva_to_gpa = nonpaging_gva_to_gpa_nested;
	} else if (is_long_mode(vcpu)) {
		g_context->nx = is_nx(vcpu);
		g_context->root_level = is_la57_mode(vcpu) ?
					PT64_ROOT_5LEVEL : PT64_ROOT_4LEVEL;
		reset_rsvds_bits_mask(vcpu, g_context);
		g_context->gva_to_gpa = paging64_gva_to_gpa_nested;
	} else if (is_pae(vcpu)) {
		g_context->nx = is_nx(vcpu);
		g_context->root_level = PT32E_ROOT_LEVEL;
		reset_rsvds_bits_mask(vcpu, g_context);
		g_context->gva_to_gpa = paging64_gva_to_gpa_nested;
	} else {
		g_context->nx = false;
		g_context->root_level = PT32_ROOT_LEVEL;
		reset_rsvds_bits_mask(vcpu, g_context);
		g_context->gva_to_gpa = paging32_gva_to_gpa_nested;
	}

	update_permission_bitmask(vcpu, g_context, false);
	update_pkru_bitmask(vcpu, g_context, false);
	update_last_nonleaf_level(vcpu, g_context);
}

static void init_kvm_mmu(struct kvm_vcpu *vcpu)
{
	if (mmu_is_nested(vcpu))
		init_kvm_nested_mmu(vcpu);
	else if (tdp_enabled)
		init_kvm_tdp_mmu(vcpu);
	else
		init_kvm_softmmu(vcpu);
}

void kvm_mmu_reset_context(struct kvm_vcpu *vcpu)
{
	kvm_mmu_unload(vcpu);
	init_kvm_mmu(vcpu);
}
EXPORT_SYMBOL_GPL(kvm_mmu_reset_context);

int kvm_mmu_load(struct kvm_vcpu *vcpu)
{
	int r;

	r = mmu_topup_memory_caches(vcpu);
	if (r)
		goto out;
	r = mmu_alloc_roots(vcpu);
	kvm_mmu_sync_roots(vcpu);
	if (r)
		goto out;
	/* set_cr3() should ensure TLB has been flushed */
	vcpu->arch.mmu.set_cr3(vcpu, vcpu->arch.mmu.root_hpa);
out:
	return r;
}
EXPORT_SYMBOL_GPL(kvm_mmu_load);

void kvm_mmu_unload(struct kvm_vcpu *vcpu)
{
	mmu_free_roots(vcpu);
	WARN_ON(VALID_PAGE(vcpu->arch.mmu.root_hpa));
}
EXPORT_SYMBOL_GPL(kvm_mmu_unload);

static void mmu_pte_write_new_pte(struct kvm_vcpu *vcpu,
				  struct kvm_mmu_page *sp, u64 *spte,
				  const void *new)
{
	if (sp->role.level != PT_PAGE_TABLE_LEVEL) {
		++vcpu->kvm->stat.mmu_pde_zapped;
		return;
        }

	++vcpu->kvm->stat.mmu_pte_updated;
	vcpu->arch.mmu.update_pte(vcpu, sp, spte, new);
}

static bool need_remote_flush(u64 old, u64 new)
{
	if (!is_shadow_present_pte(old))
		return false;
	if (!is_shadow_present_pte(new))
		return true;
	if ((old ^ new) & PT64_BASE_ADDR_MASK)
		return true;
	old ^= shadow_nx_mask;
	new ^= shadow_nx_mask;
	return (old & ~new & PT64_PERM_MASK) != 0;
}

static u64 mmu_pte_write_fetch_gpte(struct kvm_vcpu *vcpu, gpa_t *gpa,
				    const u8 *new, int *bytes)
{
	u64 gentry;
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
		r = kvm_vcpu_read_guest(vcpu, *gpa, &gentry, 8);
		if (r)
			gentry = 0;
		new = (const u8 *)&gentry;
	}

	switch (*bytes) {
	case 4:
		gentry = *(const u32 *)new;
		break;
	case 8:
		gentry = *(const u64 *)new;
		break;
	default:
		gentry = 0;
		break;
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
	if (sp->role.level == PT_PAGE_TABLE_LEVEL)
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
	pte_size = sp->role.cr4_pae ? 8 : 4;

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
	if (!sp->role.cr4_pae) {
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
	bool remote_flush, local_flush;
	union kvm_mmu_page_role mask = { };

	mask.cr0_wp = 1;
	mask.cr4_pae = 1;
	mask.nxe = 1;
	mask.smep_andnot_wp = 1;
	mask.smap_andnot_wp = 1;
	mask.smm = 1;
	mask.ad_disabled = 1;

	/*
	 * If we don't have indirect shadow pages, it means no page is
	 * write-protected, so we can exit simply.
	 */
	if (!ACCESS_ONCE(vcpu->kvm->arch.indirect_shadow_pages))
		return;

	remote_flush = local_flush = false;

	pgprintk("%s: gpa %llx bytes %d\n", __func__, gpa, bytes);

	gentry = mmu_pte_write_fetch_gpte(vcpu, &gpa, new, &bytes);

	/*
	 * No need to care whether allocation memory is successful
	 * or not since pte prefetch is skiped if it does not have
	 * enough objects in the cache.
	 */
	mmu_topup_memory_caches(vcpu);

	spin_lock(&vcpu->kvm->mmu_lock);
	++vcpu->kvm->stat.mmu_pte_write;
	kvm_mmu_audit(vcpu, AUDIT_PRE_PTE_WRITE);

	for_each_gfn_indirect_valid_sp(vcpu->kvm, sp, gfn) {
		if (detect_write_misaligned(sp, gpa, bytes) ||
		      detect_write_flooding(sp)) {
			kvm_mmu_prepare_zap_page(vcpu->kvm, sp, &invalid_list);
			++vcpu->kvm->stat.mmu_flooded;
			continue;
		}

		spte = get_written_sptes(sp, gpa, &npte);
		if (!spte)
			continue;

		local_flush = true;
		while (npte--) {
			entry = *spte;
			mmu_page_zap_pte(vcpu->kvm, sp, spte);
			if (gentry &&
			      !((sp->role.word ^ vcpu->arch.mmu.base_role.word)
			      & mask.word) && rmap_can_add(vcpu))
				mmu_pte_write_new_pte(vcpu, sp, spte, &gentry);
			if (need_remote_flush(entry, *spte))
				remote_flush = true;
			++spte;
		}
	}
	kvm_mmu_flush_or_zap(vcpu, &invalid_list, remote_flush, local_flush);
	kvm_mmu_audit(vcpu, AUDIT_POST_PTE_WRITE);
	spin_unlock(&vcpu->kvm->mmu_lock);
}

int kvm_mmu_unprotect_page_virt(struct kvm_vcpu *vcpu, gva_t gva)
{
	gpa_t gpa;
	int r;

	if (vcpu->arch.mmu.direct_map)
		return 0;

	gpa = kvm_mmu_gva_to_gpa_read(vcpu, gva, NULL);

	r = kvm_mmu_unprotect_page(vcpu->kvm, gpa >> PAGE_SHIFT);

	return r;
}
EXPORT_SYMBOL_GPL(kvm_mmu_unprotect_page_virt);

static int make_mmu_pages_available(struct kvm_vcpu *vcpu)
{
	LIST_HEAD(invalid_list);

	if (likely(kvm_mmu_available_pages(vcpu->kvm) >= KVM_MIN_FREE_MMU_PAGES))
		return 0;

	while (kvm_mmu_available_pages(vcpu->kvm) < KVM_REFILL_PAGES) {
		if (!prepare_zap_oldest_mmu_page(vcpu->kvm, &invalid_list))
			break;

		++vcpu->kvm->stat.mmu_recycled;
	}
	kvm_mmu_commit_zap_page(vcpu->kvm, &invalid_list);

	if (!kvm_mmu_available_pages(vcpu->kvm))
		return -ENOSPC;
	return 0;
}

int kvm_mmu_page_fault(struct kvm_vcpu *vcpu, gva_t cr2, u64 error_code,
		       void *insn, int insn_len)
{
	int r, emulation_type = EMULTYPE_RETRY;
	enum emulation_result er;
	bool direct = vcpu->arch.mmu.direct_map;

	/* With shadow page tables, fault_address contains a GVA or nGPA.  */
	if (vcpu->arch.mmu.direct_map) {
		vcpu->arch.gpa_available = true;
		vcpu->arch.gpa_val = cr2;
	}

	if (unlikely(error_code & PFERR_RSVD_MASK)) {
		r = handle_mmio_page_fault(vcpu, cr2, direct);
		if (r == RET_MMIO_PF_EMULATE) {
			emulation_type = 0;
			goto emulate;
		}
		if (r == RET_MMIO_PF_RETRY)
			return 1;
		if (r < 0)
			return r;
		/* Must be RET_MMIO_PF_INVALID.  */
	}

	r = vcpu->arch.mmu.page_fault(vcpu, cr2, lower_32_bits(error_code),
				      false);
	if (r < 0)
		return r;
	if (!r)
		return 1;

	/*
	 * Before emulating the instruction, check if the error code
	 * was due to a RO violation while translating the guest page.
	 * This can occur when using nested virtualization with nested
	 * paging in both guests. If true, we simply unprotect the page
	 * and resume the guest.
	 */
	if (vcpu->arch.mmu.direct_map &&
	    (error_code & PFERR_NESTED_GUEST_PAGE) == PFERR_NESTED_GUEST_PAGE) {
		kvm_mmu_unprotect_page(vcpu->kvm, gpa_to_gfn(cr2));
		return 1;
	}

	if (mmio_info_in_cache(vcpu, cr2, direct))
		emulation_type = 0;
emulate:
	er = x86_emulate_instruction(vcpu, cr2, emulation_type, insn, insn_len);

	switch (er) {
	case EMULATE_DONE:
		return 1;
	case EMULATE_USER_EXIT:
		++vcpu->stat.mmio_exits;
		/* fall through */
	case EMULATE_FAIL:
		return 0;
	default:
		BUG();
	}
}
EXPORT_SYMBOL_GPL(kvm_mmu_page_fault);

void kvm_mmu_invlpg(struct kvm_vcpu *vcpu, gva_t gva)
{
	vcpu->arch.mmu.invlpg(vcpu, gva);
	kvm_make_request(KVM_REQ_TLB_FLUSH, vcpu);
	++vcpu->stat.invlpg;
}
EXPORT_SYMBOL_GPL(kvm_mmu_invlpg);

void kvm_enable_tdp(void)
{
	tdp_enabled = true;
}
EXPORT_SYMBOL_GPL(kvm_enable_tdp);

void kvm_disable_tdp(void)
{
	tdp_enabled = false;
}
EXPORT_SYMBOL_GPL(kvm_disable_tdp);

static void free_mmu_pages(struct kvm_vcpu *vcpu)
{
	free_page((unsigned long)vcpu->arch.mmu.pae_root);
	if (vcpu->arch.mmu.lm_root != NULL)
		free_page((unsigned long)vcpu->arch.mmu.lm_root);
}

static int alloc_mmu_pages(struct kvm_vcpu *vcpu)
{
	struct page *page;
	int i;

	/*
	 * When emulating 32-bit mode, cr3 is only 32 bits even on x86_64.
	 * Therefore we need to allocate shadow page tables in the first
	 * 4GB of memory, which happens to fit the DMA32 zone.
	 */
	page = alloc_page(GFP_KERNEL | __GFP_DMA32);
	if (!page)
		return -ENOMEM;

	vcpu->arch.mmu.pae_root = page_address(page);
	for (i = 0; i < 4; ++i)
		vcpu->arch.mmu.pae_root[i] = INVALID_PAGE;

	return 0;
}

int kvm_mmu_create(struct kvm_vcpu *vcpu)
{
	vcpu->arch.walk_mmu = &vcpu->arch.mmu;
	vcpu->arch.mmu.root_hpa = INVALID_PAGE;
	vcpu->arch.mmu.translate_gpa = translate_gpa;
	vcpu->arch.nested_mmu.translate_gpa = translate_nested_gpa;

	return alloc_mmu_pages(vcpu);
}

void kvm_mmu_setup(struct kvm_vcpu *vcpu)
{
	MMU_WARN_ON(VALID_PAGE(vcpu->arch.mmu.root_hpa));

	init_kvm_mmu(vcpu);
}

static void kvm_mmu_invalidate_zap_pages_in_memslot(struct kvm *kvm,
			struct kvm_memory_slot *slot,
			struct kvm_page_track_notifier_node *node)
{
	kvm_mmu_invalidate_zap_all_pages(kvm);
}

void kvm_mmu_init_vm(struct kvm *kvm)
{
	struct kvm_page_track_notifier_node *node = &kvm->arch.mmu_sp_tracker;

	node->track_write = kvm_mmu_pte_write;
	node->track_flush_slot = kvm_mmu_invalidate_zap_pages_in_memslot;
	kvm_page_track_register_notifier(kvm, node);
}

void kvm_mmu_uninit_vm(struct kvm *kvm)
{
	struct kvm_page_track_notifier_node *node = &kvm->arch.mmu_sp_tracker;

	kvm_page_track_unregister_notifier(kvm, node);
}

/* The return value indicates if tlb flush on all vcpus is needed. */
typedef bool (*slot_level_handler) (struct kvm *kvm, struct kvm_rmap_head *rmap_head);

/* The caller should hold mmu-lock before calling this function. */
static bool
slot_handle_level_range(struct kvm *kvm, struct kvm_memory_slot *memslot,
			slot_level_handler fn, int start_level, int end_level,
			gfn_t start_gfn, gfn_t end_gfn, bool lock_flush_tlb)
{
	struct slot_rmap_walk_iterator iterator;
	bool flush = false;

	for_each_slot_rmap_range(memslot, start_level, end_level, start_gfn,
			end_gfn, &iterator) {
		if (iterator.rmap)
			flush |= fn(kvm, iterator.rmap);

		if (need_resched() || spin_needbreak(&kvm->mmu_lock)) {
			if (flush && lock_flush_tlb) {
				kvm_flush_remote_tlbs(kvm);
				flush = false;
			}
			cond_resched_lock(&kvm->mmu_lock);
		}
	}

	if (flush && lock_flush_tlb) {
		kvm_flush_remote_tlbs(kvm);
		flush = false;
	}

	return flush;
}

static bool
slot_handle_level(struct kvm *kvm, struct kvm_memory_slot *memslot,
		  slot_level_handler fn, int start_level, int end_level,
		  bool lock_flush_tlb)
{
	return slot_handle_level_range(kvm, memslot, fn, start_level,
			end_level, memslot->base_gfn,
			memslot->base_gfn + memslot->npages - 1,
			lock_flush_tlb);
}

static bool
slot_handle_all_level(struct kvm *kvm, struct kvm_memory_slot *memslot,
		      slot_level_handler fn, bool lock_flush_tlb)
{
	return slot_handle_level(kvm, memslot, fn, PT_PAGE_TABLE_LEVEL,
				 PT_MAX_HUGEPAGE_LEVEL, lock_flush_tlb);
}

static bool
slot_handle_large_level(struct kvm *kvm, struct kvm_memory_slot *memslot,
			slot_level_handler fn, bool lock_flush_tlb)
{
	return slot_handle_level(kvm, memslot, fn, PT_PAGE_TABLE_LEVEL + 1,
				 PT_MAX_HUGEPAGE_LEVEL, lock_flush_tlb);
}

static bool
slot_handle_leaf(struct kvm *kvm, struct kvm_memory_slot *memslot,
		 slot_level_handler fn, bool lock_flush_tlb)
{
	return slot_handle_level(kvm, memslot, fn, PT_PAGE_TABLE_LEVEL,
				 PT_PAGE_TABLE_LEVEL, lock_flush_tlb);
}

void kvm_zap_gfn_range(struct kvm *kvm, gfn_t gfn_start, gfn_t gfn_end)
{
	struct kvm_memslots *slots;
	struct kvm_memory_slot *memslot;
	int i;

	spin_lock(&kvm->mmu_lock);
	for (i = 0; i < KVM_ADDRESS_SPACE_NUM; i++) {
		slots = __kvm_memslots(kvm, i);
		kvm_for_each_memslot(memslot, slots) {
			gfn_t start, end;

			start = max(gfn_start, memslot->base_gfn);
			end = min(gfn_end, memslot->base_gfn + memslot->npages);
			if (start >= end)
				continue;

			slot_handle_level_range(kvm, memslot, kvm_zap_rmapp,
						PT_PAGE_TABLE_LEVEL, PT_MAX_HUGEPAGE_LEVEL,
						start, end - 1, true);
		}
	}

	spin_unlock(&kvm->mmu_lock);
}

static bool slot_rmap_write_protect(struct kvm *kvm,
				    struct kvm_rmap_head *rmap_head)
{
	return __rmap_write_protect(kvm, rmap_head, false);
}

void kvm_mmu_slot_remove_write_access(struct kvm *kvm,
				      struct kvm_memory_slot *memslot)
{
	bool flush;

	spin_lock(&kvm->mmu_lock);
	flush = slot_handle_all_level(kvm, memslot, slot_rmap_write_protect,
				      false);
	spin_unlock(&kvm->mmu_lock);

	/*
	 * kvm_mmu_slot_remove_write_access() and kvm_vm_ioctl_get_dirty_log()
	 * which do tlb flush out of mmu-lock should be serialized by
	 * kvm->slots_lock otherwise tlb flush would be missed.
	 */
	lockdep_assert_held(&kvm->slots_lock);

	/*
	 * We can flush all the TLBs out of the mmu lock without TLB
	 * corruption since we just change the spte from writable to
	 * readonly so that we only need to care the case of changing
	 * spte from present to present (changing the spte from present
	 * to nonpresent will flush all the TLBs immediately), in other
	 * words, the only case we care is mmu_spte_update() where we
	 * haved checked SPTE_HOST_WRITEABLE | SPTE_MMU_WRITEABLE
	 * instead of PT_WRITABLE_MASK, that means it does not depend
	 * on PT_WRITABLE_MASK anymore.
	 */
	if (flush)
		kvm_flush_remote_tlbs(kvm);
}

static bool kvm_mmu_zap_collapsible_spte(struct kvm *kvm,
					 struct kvm_rmap_head *rmap_head)
{
	u64 *sptep;
	struct rmap_iterator iter;
	int need_tlb_flush = 0;
	kvm_pfn_t pfn;
	struct kvm_mmu_page *sp;

restart:
	for_each_rmap_spte(rmap_head, &iter, sptep) {
		sp = page_header(__pa(sptep));
		pfn = spte_to_pfn(*sptep);

		/*
		 * We cannot do huge page mapping for indirect shadow pages,
		 * which are found on the last rmap (level = 1) when not using
		 * tdp; such shadow pages are synced with the page table in
		 * the guest, and the guest page table is using 4K page size
		 * mapping if the indirect sp has level = 1.
		 */
		if (sp->role.direct &&
			!kvm_is_reserved_pfn(pfn) &&
			PageTransCompoundMap(pfn_to_page(pfn))) {
			drop_spte(kvm, sptep);
			need_tlb_flush = 1;
			goto restart;
		}
	}

	return need_tlb_flush;
}

void kvm_mmu_zap_collapsible_sptes(struct kvm *kvm,
				   const struct kvm_memory_slot *memslot)
{
	/* FIXME: const-ify all uses of struct kvm_memory_slot.  */
	spin_lock(&kvm->mmu_lock);
	slot_handle_leaf(kvm, (struct kvm_memory_slot *)memslot,
			 kvm_mmu_zap_collapsible_spte, true);
	spin_unlock(&kvm->mmu_lock);
}

void kvm_mmu_slot_leaf_clear_dirty(struct kvm *kvm,
				   struct kvm_memory_slot *memslot)
{
	bool flush;

	spin_lock(&kvm->mmu_lock);
	flush = slot_handle_leaf(kvm, memslot, __rmap_clear_dirty, false);
	spin_unlock(&kvm->mmu_lock);

	lockdep_assert_held(&kvm->slots_lock);

	/*
	 * It's also safe to flush TLBs out of mmu lock here as currently this
	 * function is only used for dirty logging, in which case flushing TLB
	 * out of mmu lock also guarantees no dirty pages will be lost in
	 * dirty_bitmap.
	 */
	if (flush)
		kvm_flush_remote_tlbs(kvm);
}
EXPORT_SYMBOL_GPL(kvm_mmu_slot_leaf_clear_dirty);

void kvm_mmu_slot_largepage_remove_write_access(struct kvm *kvm,
					struct kvm_memory_slot *memslot)
{
	bool flush;

	spin_lock(&kvm->mmu_lock);
	flush = slot_handle_large_level(kvm, memslot, slot_rmap_write_protect,
					false);
	spin_unlock(&kvm->mmu_lock);

	/* see kvm_mmu_slot_remove_write_access */
	lockdep_assert_held(&kvm->slots_lock);

	if (flush)
		kvm_flush_remote_tlbs(kvm);
}
EXPORT_SYMBOL_GPL(kvm_mmu_slot_largepage_remove_write_access);

void kvm_mmu_slot_set_dirty(struct kvm *kvm,
			    struct kvm_memory_slot *memslot)
{
	bool flush;

	spin_lock(&kvm->mmu_lock);
	flush = slot_handle_all_level(kvm, memslot, __rmap_set_dirty, false);
	spin_unlock(&kvm->mmu_lock);

	lockdep_assert_held(&kvm->slots_lock);

	/* see kvm_mmu_slot_leaf_clear_dirty */
	if (flush)
		kvm_flush_remote_tlbs(kvm);
}
EXPORT_SYMBOL_GPL(kvm_mmu_slot_set_dirty);

#define BATCH_ZAP_PAGES	10
static void kvm_zap_obsolete_pages(struct kvm *kvm)
{
	struct kvm_mmu_page *sp, *node;
	int batch = 0;

restart:
	list_for_each_entry_safe_reverse(sp, node,
	      &kvm->arch.active_mmu_pages, link) {
		int ret;

		/*
		 * No obsolete page exists before new created page since
		 * active_mmu_pages is the FIFO list.
		 */
		if (!is_obsolete_sp(kvm, sp))
			break;

		/*
		 * Since we are reversely walking the list and the invalid
		 * list will be moved to the head, skip the invalid page
		 * can help us to avoid the infinity list walking.
		 */
		if (sp->role.invalid)
			continue;

		/*
		 * Need not flush tlb since we only zap the sp with invalid
		 * generation number.
		 */
		if (batch >= BATCH_ZAP_PAGES &&
		      cond_resched_lock(&kvm->mmu_lock)) {
			batch = 0;
			goto restart;
		}

		ret = kvm_mmu_prepare_zap_page(kvm, sp,
				&kvm->arch.zapped_obsolete_pages);
		batch += ret;

		if (ret)
			goto restart;
	}

	/*
	 * Should flush tlb before free page tables since lockless-walking
	 * may use the pages.
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
void kvm_mmu_invalidate_zap_all_pages(struct kvm *kvm)
{
	spin_lock(&kvm->mmu_lock);
	trace_kvm_mmu_invalidate_zap_all_pages(kvm);
	kvm->arch.mmu_valid_gen++;

	/*
	 * Notify all vcpus to reload its shadow page table
	 * and flush TLB. Then all vcpus will switch to new
	 * shadow page table with the new mmu_valid_gen.
	 *
	 * Note: we should do this under the protection of
	 * mmu-lock, otherwise, vcpu would purge shadow page
	 * but miss tlb flush.
	 */
	kvm_reload_remote_mmus(kvm);

	kvm_zap_obsolete_pages(kvm);
	spin_unlock(&kvm->mmu_lock);
}

static bool kvm_has_zapped_obsolete_pages(struct kvm *kvm)
{
	return unlikely(!list_empty_careful(&kvm->arch.zapped_obsolete_pages));
}

void kvm_mmu_invalidate_mmio_sptes(struct kvm *kvm, struct kvm_memslots *slots)
{
	/*
	 * The very rare case: if the generation-number is round,
	 * zap all shadow pages.
	 */
	if (unlikely((slots->generation & MMIO_GEN_MASK) == 0)) {
		kvm_debug_ratelimited("kvm: zapping shadow pages for mmio generation wraparound\n");
		kvm_mmu_invalidate_zap_all_pages(kvm);
	}
}

static unsigned long
mmu_shrink_scan(struct shrinker *shrink, struct shrink_control *sc)
{
	struct kvm *kvm;
	int nr_to_scan = sc->nr_to_scan;
	unsigned long freed = 0;

	spin_lock(&kvm_lock);

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
		spin_lock(&kvm->mmu_lock);

		if (kvm_has_zapped_obsolete_pages(kvm)) {
			kvm_mmu_commit_zap_page(kvm,
			      &kvm->arch.zapped_obsolete_pages);
			goto unlock;
		}

		if (prepare_zap_oldest_mmu_page(kvm, &invalid_list))
			freed++;
		kvm_mmu_commit_zap_page(kvm, &invalid_list);

unlock:
		spin_unlock(&kvm->mmu_lock);
		srcu_read_unlock(&kvm->srcu, idx);

		/*
		 * unfair on small ones
		 * per-vm shrinkers cry out
		 * sadness comes quickly
		 */
		list_move_tail(&kvm->vm_list, &vm_list);
		break;
	}

	spin_unlock(&kvm_lock);
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
	if (pte_list_desc_cache)
		kmem_cache_destroy(pte_list_desc_cache);
	if (mmu_page_header_cache)
		kmem_cache_destroy(mmu_page_header_cache);
}

int kvm_mmu_module_init(void)
{
	kvm_mmu_clear_all_pte_masks();

	pte_list_desc_cache = kmem_cache_create("pte_list_desc",
					    sizeof(struct pte_list_desc),
					    0, 0, NULL);
	if (!pte_list_desc_cache)
		goto nomem;

	mmu_page_header_cache = kmem_cache_create("kvm_mmu_page_header",
						  sizeof(struct kvm_mmu_page),
						  0, 0, NULL);
	if (!mmu_page_header_cache)
		goto nomem;

	if (percpu_counter_init(&kvm_total_used_mmu_pages, 0, GFP_KERNEL))
		goto nomem;

	register_shrinker(&mmu_shrinker);

	return 0;

nomem:
	mmu_destroy_caches();
	return -ENOMEM;
}

/*
 * Caculate mmu pages needed for kvm.
 */
unsigned int kvm_mmu_calculate_mmu_pages(struct kvm *kvm)
{
	unsigned int nr_mmu_pages;
	unsigned int  nr_pages = 0;
	struct kvm_memslots *slots;
	struct kvm_memory_slot *memslot;
	int i;

	for (i = 0; i < KVM_ADDRESS_SPACE_NUM; i++) {
		slots = __kvm_memslots(kvm, i);

		kvm_for_each_memslot(memslot, slots)
			nr_pages += memslot->npages;
	}

	nr_mmu_pages = nr_pages * KVM_PERMILLE_MMU_PAGES / 1000;
	nr_mmu_pages = max(nr_mmu_pages,
			   (unsigned int) KVM_MIN_ALLOC_MMU_PAGES);

	return nr_mmu_pages;
}

void kvm_mmu_destroy(struct kvm_vcpu *vcpu)
{
	kvm_mmu_unload(vcpu);
	free_mmu_pages(vcpu);
	mmu_free_memory_caches(vcpu);
}

void kvm_mmu_module_exit(void)
{
	mmu_destroy_caches();
	percpu_counter_destroy(&kvm_total_used_mmu_pages);
	unregister_shrinker(&mmu_shrinker);
	mmu_audit_disable();
}