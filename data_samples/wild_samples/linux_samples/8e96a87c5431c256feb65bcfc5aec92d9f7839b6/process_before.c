	if (!current->thread.regs) {
		struct pt_regs *regs = task_stack_page(current) + THREAD_SIZE;
		current->thread.regs = regs - 1;
	}

	memset(regs->gpr, 0, sizeof(regs->gpr));
	regs->ctr = 0;
	regs->link = 0;
	regs->xer = 0;
	regs->ccr = 0;
	regs->gpr[1] = sp;

	/*
	 * We have just cleared all the nonvolatile GPRs, so make
	 * FULL_REGS(regs) return true.  This is necessary to allow
	 * ptrace to examine the thread immediately after exec.
	 */
	regs->trap &= ~1UL;

#ifdef CONFIG_PPC32
	regs->mq = 0;
	regs->nip = start;
	regs->msr = MSR_USER;
#else
	if (!is_32bit_task()) {
		unsigned long entry;

		if (is_elf2_task()) {
			/* Look ma, no function descriptors! */
			entry = start;

			/*
			 * Ulrich says:
			 *   The latest iteration of the ABI requires that when
			 *   calling a function (at its global entry point),
			 *   the caller must ensure r12 holds the entry point
			 *   address (so that the function can quickly
			 *   establish addressability).
			 */
			regs->gpr[12] = start;
			/* Make sure that's restored on entry to userspace. */
			set_thread_flag(TIF_RESTOREALL);
		} else {
			unsigned long toc;

			/* start is a relocated pointer to the function
			 * descriptor for the elf _start routine.  The first
			 * entry in the function descriptor is the entry
			 * address of _start and the second entry is the TOC
			 * value we need to use.
			 */
			__get_user(entry, (unsigned long __user *)start);
			__get_user(toc, (unsigned long __user *)start+1);

			/* Check whether the e_entry function descriptor entries
			 * need to be relocated before we can use them.
			 */
			if (load_addr != 0) {
				entry += load_addr;
				toc   += load_addr;
			}
			regs->gpr[2] = toc;
		}
		regs->nip = entry;
		regs->msr = MSR_USER64;
	} else {
		regs->nip = start;
		regs->gpr[2] = 0;
		regs->msr = MSR_USER32;
	}
#endif
#ifdef CONFIG_VSX
	current->thread.used_vsr = 0;
#endif
	memset(&current->thread.fp_state, 0, sizeof(current->thread.fp_state));
	current->thread.fp_save_area = NULL;
#ifdef CONFIG_ALTIVEC
	memset(&current->thread.vr_state, 0, sizeof(current->thread.vr_state));
	current->thread.vr_state.vscr.u[3] = 0x00010000; /* Java mode disabled */
	current->thread.vr_save_area = NULL;
	current->thread.vrsave = 0;
	current->thread.used_vr = 0;
#endif /* CONFIG_ALTIVEC */
#ifdef CONFIG_SPE
	memset(current->thread.evr, 0, sizeof(current->thread.evr));
	current->thread.acc = 0;
	current->thread.spefscr = 0;
	current->thread.used_spe = 0;
#endif /* CONFIG_SPE */
#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
	if (cpu_has_feature(CPU_FTR_TM))
		regs->msr |= MSR_TM;
	current->thread.tm_tfhar = 0;
	current->thread.tm_texasr = 0;
	current->thread.tm_tfiar = 0;
#endif /* CONFIG_PPC_TRANSACTIONAL_MEM */
}