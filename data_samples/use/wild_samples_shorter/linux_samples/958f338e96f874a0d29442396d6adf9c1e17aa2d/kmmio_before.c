
static void clear_pmd_presence(pmd_t *pmd, bool clear, pmdval_t *old)
{
	pmdval_t v = pmd_val(*pmd);
	if (clear) {
		*old = v & _PAGE_PRESENT;
		v &= ~_PAGE_PRESENT;
	} else	/* presume this has been called with clear==true previously */
		v |= *old;
	set_pmd(pmd, __pmd(v));
}

static void clear_pte_presence(pte_t *pte, bool clear, pteval_t *old)
{
	pteval_t v = pte_val(*pte);
	if (clear) {
		*old = v & _PAGE_PRESENT;
		v &= ~_PAGE_PRESENT;
	} else	/* presume this has been called with clear==true previously */
		v |= *old;
	set_pte_atomic(pte, __pte(v));
}

static int clear_page_presence(struct kmmio_fault_page *f, bool clear)
{