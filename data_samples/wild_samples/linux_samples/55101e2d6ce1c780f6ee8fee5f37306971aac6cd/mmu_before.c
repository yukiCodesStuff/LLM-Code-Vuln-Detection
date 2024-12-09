	for (pte = 0; pte < 64; pte++) {
		int rwx_bits = pte & 7;
		int mt = pte >> 3;
		if (mt == 0x2 || mt == 0x3 || mt == 0x7 ||
				rwx_bits == 0x2 || rwx_bits == 0x6 ||
				(rwx_bits == 0x4 && !execonly))
			context->bad_mt_xwr |= (1ull << pte);
	}
}

static void update_permission_bitmask(struct kvm_vcpu *vcpu,
		struct kvm_mmu *mmu, bool ept)
{
	unsigned bit, byte, pfec;
	u8 map;
	bool fault, x, w, u, wf, uf, ff, smep;

	smep = kvm_read_cr4_bits(vcpu, X86_CR4_SMEP);
	for (byte = 0; byte < ARRAY_SIZE(mmu->permissions); ++byte) {
		pfec = byte << 1;
		map = 0;
		wf = pfec & PFERR_WRITE_MASK;
		uf = pfec & PFERR_USER_MASK;
		ff = pfec & PFERR_FETCH_MASK;
		for (bit = 0; bit < 8; ++bit) {
			x = bit & ACC_EXEC_MASK;
			w = bit & ACC_WRITE_MASK;
			u = bit & ACC_USER_MASK;

			if (!ept) {
				/* Not really needed: !nx will cause pte.nx to fault */
				x |= !mmu->nx;
				/* Allow supervisor writes if !cr0.wp */
				w |= !is_write_protection(vcpu) && !uf;
				/* Disallow supervisor fetches of user code if cr4.smep */
				x &= !(smep && u && !uf);
			} else
				/* Not really needed: no U/S accesses on ept  */
				u = 1;

			fault = (ff && !x) || (uf && !u) || (wf && !w);
			map |= fault << bit;
		}
		mmu->permissions[byte] = map;
	}
}
			if (!ept) {
				/* Not really needed: !nx will cause pte.nx to fault */
				x |= !mmu->nx;
				/* Allow supervisor writes if !cr0.wp */
				w |= !is_write_protection(vcpu) && !uf;
				/* Disallow supervisor fetches of user code if cr4.smep */
				x &= !(smep && u && !uf);
			} else
				/* Not really needed: no U/S accesses on ept  */
				u = 1;

			fault = (ff && !x) || (uf && !u) || (wf && !w);
			map |= fault << bit;
		}
		mmu->permissions[byte] = map;
	}
}

static void update_last_pte_bitmap(struct kvm_vcpu *vcpu, struct kvm_mmu *mmu)
{
	u8 map;
	unsigned level, root_level = mmu->root_level;
	const unsigned ps_set_index = 1 << 2;  /* bit 2 of index: ps */

	if (root_level == PT32E_ROOT_LEVEL)
		--root_level;
	/* PT_PAGE_TABLE_LEVEL always terminates */
	map = 1 | (1 << ps_set_index);
	for (level = PT_DIRECTORY_LEVEL; level <= root_level; ++level) {
		if (level <= PT_PDPE_LEVEL
		    && (mmu->root_level >= PT32E_ROOT_LEVEL || is_pse(vcpu)))
			map |= 1 << (ps_set_index | (level - 1));
	}
	mmu->last_pte_bitmap = map;
}

static void paging64_init_context_common(struct kvm_vcpu *vcpu,
					 struct kvm_mmu *context,
					 int level)
{
	context->nx = is_nx(vcpu);
	context->root_level = level;

	reset_rsvds_bits_mask(vcpu, context);
	update_permission_bitmask(vcpu, context, false);
	update_last_pte_bitmap(vcpu, context);

	ASSERT(is_pae(vcpu));
	context->page_fault = paging64_page_fault;
	context->gva_to_gpa = paging64_gva_to_gpa;
	context->sync_page = paging64_sync_page;
	context->invlpg = paging64_invlpg;
	context->update_pte = paging64_update_pte;
	context->shadow_root_level = level;
	context->root_hpa = INVALID_PAGE;
	context->direct_map = false;
}

static void paging64_init_context(struct kvm_vcpu *vcpu,
				  struct kvm_mmu *context)
{
	paging64_init_context_common(vcpu, context, PT64_ROOT_LEVEL);
}

static void paging32_init_context(struct kvm_vcpu *vcpu,
				  struct kvm_mmu *context)
{
	context->nx = false;
	context->root_level = PT32_ROOT_LEVEL;

	reset_rsvds_bits_mask(vcpu, context);
	update_permission_bitmask(vcpu, context, false);
	update_last_pte_bitmap(vcpu, context);

	context->page_fault = paging32_page_fault;
	context->gva_to_gpa = paging32_gva_to_gpa;
	context->sync_page = paging32_sync_page;
	context->invlpg = paging32_invlpg;
	context->update_pte = paging32_update_pte;
	context->shadow_root_level = PT32E_ROOT_LEVEL;
	context->root_hpa = INVALID_PAGE;
	context->direct_map = false;
}

static void paging32E_init_context(struct kvm_vcpu *vcpu,
				   struct kvm_mmu *context)
{
	paging64_init_context_common(vcpu, context, PT32E_ROOT_LEVEL);
}

static void init_kvm_tdp_mmu(struct kvm_vcpu *vcpu)
{
	struct kvm_mmu *context = vcpu->arch.walk_mmu;

	context->base_role.word = 0;
	context->page_fault = tdp_page_fault;
	context->sync_page = nonpaging_sync_page;
	context->invlpg = nonpaging_invlpg;
	context->update_pte = nonpaging_update_pte;
	context->shadow_root_level = kvm_x86_ops->get_tdp_level();
	context->root_hpa = INVALID_PAGE;
	context->direct_map = true;
	context->set_cr3 = kvm_x86_ops->set_tdp_cr3;
	context->get_cr3 = get_cr3;
	context->get_pdptr = kvm_pdptr_read;
	context->inject_page_fault = kvm_inject_page_fault;

	if (!is_paging(vcpu)) {
		context->nx = false;
		context->gva_to_gpa = nonpaging_gva_to_gpa;
		context->root_level = 0;
	} else if (is_long_mode(vcpu)) {
		context->nx = is_nx(vcpu);
		context->root_level = PT64_ROOT_LEVEL;
		reset_rsvds_bits_mask(vcpu, context);
		context->gva_to_gpa = paging64_gva_to_gpa;
	} else if (is_pae(vcpu)) {
		context->nx = is_nx(vcpu);
		context->root_level = PT32E_ROOT_LEVEL;
		reset_rsvds_bits_mask(vcpu, context);
		context->gva_to_gpa = paging64_gva_to_gpa;
	} else {
		context->nx = false;
		context->root_level = PT32_ROOT_LEVEL;
		reset_rsvds_bits_mask(vcpu, context);
		context->gva_to_gpa = paging32_gva_to_gpa;
	}

	update_permission_bitmask(vcpu, context, false);
	update_last_pte_bitmap(vcpu, context);
}