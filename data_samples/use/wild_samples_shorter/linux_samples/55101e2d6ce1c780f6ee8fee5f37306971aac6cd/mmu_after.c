	}
}

void update_permission_bitmask(struct kvm_vcpu *vcpu,
		struct kvm_mmu *mmu, bool ept)
{
	unsigned bit, byte, pfec;
	u8 map;
	bool fault, x, w, u, wf, uf, ff, smapf, cr4_smap, cr4_smep, smap = 0;

	cr4_smep = kvm_read_cr4_bits(vcpu, X86_CR4_SMEP);
	cr4_smap = kvm_read_cr4_bits(vcpu, X86_CR4_SMAP);
	for (byte = 0; byte < ARRAY_SIZE(mmu->permissions); ++byte) {
		pfec = byte << 1;
		map = 0;
		wf = pfec & PFERR_WRITE_MASK;
		uf = pfec & PFERR_USER_MASK;
		ff = pfec & PFERR_FETCH_MASK;
		/*
		 * PFERR_RSVD_MASK bit is set in PFEC if the access is not
		 * subject to SMAP restrictions, and cleared otherwise. The
		 * bit is only meaningful if the SMAP bit is set in CR4.
		 */
		smapf = !(pfec & PFERR_RSVD_MASK);
		for (bit = 0; bit < 8; ++bit) {
			x = bit & ACC_EXEC_MASK;
			w = bit & ACC_WRITE_MASK;
			u = bit & ACC_USER_MASK;
				/* Allow supervisor writes if !cr0.wp */
				w |= !is_write_protection(vcpu) && !uf;
				/* Disallow supervisor fetches of user code if cr4.smep */
				x &= !(cr4_smep && u && !uf);

				/*
				 * SMAP:kernel-mode data accesses from user-mode
				 * mappings should fault. A fault is considered
				 * as a SMAP violation if all of the following
				 * conditions are ture:
				 *   - X86_CR4_SMAP is set in CR4
				 *   - An user page is accessed
				 *   - Page fault in kernel mode
				 *   - if CPL = 3 or X86_EFLAGS_AC is clear
				 *
				 *   Here, we cover the first three conditions.
				 *   The fourth is computed dynamically in
				 *   permission_fault() and is in smapf.
				 *
				 *   Also, SMAP does not affect instruction
				 *   fetches, add the !ff check here to make it
				 *   clearer.
				 */
				smap = cr4_smap && u && !uf && !ff;
			} else
				/* Not really needed: no U/S accesses on ept  */
				u = 1;

			fault = (ff && !x) || (uf && !u) || (wf && !w) ||
				(smapf && smap);
			map |= fault << bit;
		}
		mmu->permissions[byte] = map;
	}