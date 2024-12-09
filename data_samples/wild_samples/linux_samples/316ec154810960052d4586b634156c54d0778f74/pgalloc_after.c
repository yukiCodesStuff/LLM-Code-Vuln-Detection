{
	struct mm_struct *mm = arg;

	/* we must change all active ASCEs to avoid the creation of new TLBs */
	if (current->active_mm == mm) {
		S390_lowcore.user_asce = mm->context.asce;
		if (current->thread.mm_segment == USER_DS) {
			__ctl_load(S390_lowcore.user_asce, 1, 1);
			/* Mark user-ASCE present in CR1 */
			clear_cpu_flag(CIF_ASCE_PRIMARY);
		}
		if (current->thread.mm_segment == USER_DS_SACF) {
			__ctl_load(S390_lowcore.user_asce, 7, 7);
			/* enable_sacf_uaccess does all or nothing */
			WARN_ON(!test_cpu_flag(CIF_ASCE_SECONDARY));
		}
	}