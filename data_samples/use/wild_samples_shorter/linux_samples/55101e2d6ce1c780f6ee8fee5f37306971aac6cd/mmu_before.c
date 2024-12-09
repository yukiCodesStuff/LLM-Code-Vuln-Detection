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