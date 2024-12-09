
}

static void walk_pte(struct pg_state *st, pmd_t *pmdp, unsigned long start,
		     unsigned long end)
{
	unsigned long addr = start;
	pte_t *ptep = pte_offset_kernel(pmdp, start);

	do {
		note_page(st, addr, 4, READ_ONCE(pte_val(*ptep)));
	} while (ptep++, addr += PAGE_SIZE, addr != end);
}

static void walk_pmd(struct pg_state *st, pud_t *pudp, unsigned long start,
		     unsigned long end)
{
	unsigned long next, addr = start;
	pmd_t *pmdp = pmd_offset(pudp, start);

	do {
		pmd_t pmd = READ_ONCE(*pmdp);
		next = pmd_addr_end(addr, end);

		if (pmd_none(pmd) || pmd_sect(pmd)) {
			note_page(st, addr, 3, pmd_val(pmd));
		} else {
			BUG_ON(pmd_bad(pmd));
			walk_pte(st, pmdp, addr, next);
		}
	} while (pmdp++, addr = next, addr != end);
}

static void walk_pud(struct pg_state *st, pgd_t *pgdp, unsigned long start,
		     unsigned long end)
{
	unsigned long next, addr = start;
	pud_t *pudp = pud_offset(pgdp, start);

	do {
		pud_t pud = READ_ONCE(*pudp);
		next = pud_addr_end(addr, end);

		if (pud_none(pud) || pud_sect(pud)) {
			note_page(st, addr, 2, pud_val(pud));
		} else {
			BUG_ON(pud_bad(pud));
			walk_pmd(st, pudp, addr, next);
		}
	} while (pudp++, addr = next, addr != end);
}

static void walk_pgd(struct pg_state *st, struct mm_struct *mm,
		     unsigned long start)
{
	unsigned long end = (start < TASK_SIZE_64) ? TASK_SIZE_64 : 0;
	unsigned long next, addr = start;
	pgd_t *pgdp = pgd_offset(mm, start);

	do {
		pgd_t pgd = READ_ONCE(*pgdp);
		next = pgd_addr_end(addr, end);

		if (pgd_none(pgd)) {
			note_page(st, addr, 1, pgd_val(pgd));
		} else {
			BUG_ON(pgd_bad(pgd));
			walk_pud(st, pgdp, addr, next);
		}
	} while (pgdp++, addr = next, addr != end);
}

void ptdump_walk_pgd(struct seq_file *m, struct ptdump_info *info)
{