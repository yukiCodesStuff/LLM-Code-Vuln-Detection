	if (addr >= st->marker[1].start_address) {
		st->marker++;
		pt_dump_seq_printf(st->seq, "---[ %s ]---\n", st->marker->name);
	}

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