
	return 0;
}
EXPORT_SYMBOL_GPL(ksm_madvise);

int __ksm_enter(struct mm_struct *mm)
{
	struct mm_slot *mm_slot;