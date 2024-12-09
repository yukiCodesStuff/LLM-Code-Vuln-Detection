	if (addr >= st->marker[1].start_address) {
		st->marker++;
		pt_dump_seq_printf(st->seq, "---[ %s ]---\n", st->marker->name);
	}

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