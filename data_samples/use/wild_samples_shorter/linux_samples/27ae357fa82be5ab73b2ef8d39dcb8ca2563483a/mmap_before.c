	/* mm's last user has gone, and its about to be pulled down */
	mmu_notifier_release(mm);

	if (mm->locked_vm) {
		vma = mm->mmap;
		while (vma) {
			if (vma->vm_flags & VM_LOCKED)
	/* update_hiwater_rss(mm) here? but nobody should be looking */
	/* Use -1 here to ensure all VMAs in the mm are unmapped */
	unmap_vmas(&tlb, vma, 0, -1);

	if (unlikely(mm_is_oom_victim(mm))) {
		/*
		 * Wait for oom_reap_task() to stop working on this
		 * mm. Because MMF_OOM_SKIP is already set before
		 * calling down_read(), oom_reap_task() will not run
		 * on this "mm" post up_write().
		 *
		 * mm_is_oom_victim() cannot be set from under us
		 * either because victim->mm is already set to NULL
		 * under task_lock before calling mmput and oom_mm is
		 * set not NULL by the OOM killer only if victim->mm
		 * is found not NULL while holding the task_lock.
		 */
		set_bit(MMF_OOM_SKIP, &mm->flags);
		down_write(&mm->mmap_sem);
		up_write(&mm->mmap_sem);
	}
	free_pgtables(&tlb, vma, FIRST_USER_ADDRESS, USER_PGTABLES_CEILING);
	tlb_finish_mmu(&tlb, 0, -1);

	/*