
static void clear_pmd_presence(pmd_t *pmd, bool clear, pmdval_t *old)
{
	pmd_t new_pmd;
	pmdval_t v = pmd_val(*pmd);
	if (clear) {
		*old = v;
		new_pmd = pmd_mknotpresent(*pmd);
	} else {
		/* Presume this has been called with clear==true previously */
		new_pmd = __pmd(*old);
	}
	set_pmd(pmd, new_pmd);
}

static void clear_pte_presence(pte_t *pte, bool clear, pteval_t *old)
{
	pteval_t v = pte_val(*pte);
	if (clear) {
		*old = v;
		/* Nothing should care about address */
		pte_clear(&init_mm, 0, pte);
	} else {
		/* Presume this has been called with clear==true previously */
		set_pte_atomic(pte, __pte(*old));
	}
}

static int clear_page_presence(struct kmmio_fault_page *f, bool clear)
{