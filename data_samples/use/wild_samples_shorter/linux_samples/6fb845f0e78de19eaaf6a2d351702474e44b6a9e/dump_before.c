
}

static void walk_pte(struct pg_state *st, pmd_t *pmdp, unsigned long start)
{
	pte_t *ptep = pte_offset_kernel(pmdp, 0UL);
	unsigned long addr;
	unsigned i;

	for (i = 0; i < PTRS_PER_PTE; i++, ptep++) {
		addr = start + i * PAGE_SIZE;
		note_page(st, addr, 4, READ_ONCE(pte_val(*ptep)));
	}
}

static void walk_pmd(struct pg_state *st, pud_t *pudp, unsigned long start)
{
	pmd_t *pmdp = pmd_offset(pudp, 0UL);
	unsigned long addr;
	unsigned i;

	for (i = 0; i < PTRS_PER_PMD; i++, pmdp++) {
		pmd_t pmd = READ_ONCE(*pmdp);

		addr = start + i * PMD_SIZE;
		if (pmd_none(pmd) || pmd_sect(pmd)) {
			note_page(st, addr, 3, pmd_val(pmd));
		} else {
			BUG_ON(pmd_bad(pmd));
			walk_pte(st, pmdp, addr);
		}
	}
}

static void walk_pud(struct pg_state *st, pgd_t *pgdp, unsigned long start)
{
	pud_t *pudp = pud_offset(pgdp, 0UL);
	unsigned long addr;
	unsigned i;

	for (i = 0; i < PTRS_PER_PUD; i++, pudp++) {
		pud_t pud = READ_ONCE(*pudp);

		addr = start + i * PUD_SIZE;
		if (pud_none(pud) || pud_sect(pud)) {
			note_page(st, addr, 2, pud_val(pud));
		} else {
			BUG_ON(pud_bad(pud));
			walk_pmd(st, pudp, addr);
		}
	}
}

static void walk_pgd(struct pg_state *st, struct mm_struct *mm,
		     unsigned long start)
{
	pgd_t *pgdp = pgd_offset(mm, 0UL);
	unsigned i;
	unsigned long addr;

	for (i = 0; i < PTRS_PER_PGD; i++, pgdp++) {
		pgd_t pgd = READ_ONCE(*pgdp);

		addr = start + i * PGDIR_SIZE;
		if (pgd_none(pgd)) {
			note_page(st, addr, 1, pgd_val(pgd));
		} else {
			BUG_ON(pgd_bad(pgd));
			walk_pud(st, pgdp, addr);
		}
	}
}

void ptdump_walk_pgd(struct seq_file *m, struct ptdump_info *info)
{