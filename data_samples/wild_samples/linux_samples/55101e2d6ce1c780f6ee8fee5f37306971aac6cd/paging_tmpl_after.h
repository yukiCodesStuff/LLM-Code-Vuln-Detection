					             walker->level))) {
			errcode |= PFERR_RSVD_MASK | PFERR_PRESENT_MASK;
			goto error;
		}

		accessed_dirty &= pte;
		pte_access = pt_access & FNAME(gpte_access)(vcpu, pte);

		walker->ptes[walker->level - 1] = pte;
	} while (!is_last_gpte(mmu, walker->level, pte));

	if (unlikely(permission_fault(vcpu, mmu, pte_access, access))) {
		errcode |= PFERR_PRESENT_MASK;
		goto error;
	}