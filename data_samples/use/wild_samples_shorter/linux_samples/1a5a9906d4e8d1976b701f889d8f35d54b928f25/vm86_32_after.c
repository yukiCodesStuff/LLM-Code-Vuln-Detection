	spinlock_t *ptl;
	int i;

	down_write(&mm->mmap_sem);
	pgd = pgd_offset(mm, 0xA0000);
	if (pgd_none_or_clear_bad(pgd))
		goto out;
	pud = pud_offset(pgd, 0xA0000);
	}
	pte_unmap_unlock(pte, ptl);
out:
	up_write(&mm->mmap_sem);
	flush_tlb();
}

