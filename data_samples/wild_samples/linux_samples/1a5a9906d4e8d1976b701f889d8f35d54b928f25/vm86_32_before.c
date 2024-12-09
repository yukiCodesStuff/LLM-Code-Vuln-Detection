{
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;
	spinlock_t *ptl;
	int i;

	pgd = pgd_offset(mm, 0xA0000);
	if (pgd_none_or_clear_bad(pgd))
		goto out;
	pud = pud_offset(pgd, 0xA0000);
	if (pud_none_or_clear_bad(pud))
		goto out;
	pmd = pmd_offset(pud, 0xA0000);
	split_huge_page_pmd(mm, pmd);
	if (pmd_none_or_clear_bad(pmd))
		goto out;
	pte = pte_offset_map_lock(mm, pmd, 0xA0000, &ptl);
	for (i = 0; i < 32; i++) {
		if (pte_present(*pte))
			set_pte(pte, pte_wrprotect(*pte));
		pte++;
	}
	for (i = 0; i < 32; i++) {
		if (pte_present(*pte))
			set_pte(pte, pte_wrprotect(*pte));
		pte++;
	}
	pte_unmap_unlock(pte, ptl);
out:
	flush_tlb();
}



static int do_vm86_irq_handling(int subfunction, int irqnumber);
static void do_sys_vm86(struct kernel_vm86_struct *info, struct task_struct *tsk);

int sys_vm86old(struct vm86_struct __user *v86, struct pt_regs *regs)
{
	struct kernel_vm86_struct info; /* declare this _on top_,
					 * this avoids wasting of stack space.
					 * This remains on the stack until we
					 * return to 32 bit user space.
					 */
	struct task_struct *tsk;
	int tmp, ret = -EPERM;

	tsk = current;
	if (tsk->thread.saved_sp0)
		goto out;
	tmp = copy_vm86_regs_from_user(&info.regs, &v86->regs,
				       offsetof(struct kernel_vm86_struct, vm86plus) -
				       sizeof(info.regs));
	ret = -EFAULT;
	if (tmp)
		goto out;
	memset(&info.vm86plus, 0, (int)&info.regs32 - (int)&info.vm86plus);
	info.regs32 = regs;
	tsk->thread.vm86_info = v86;
	do_sys_vm86(&info, tsk);
	ret = 0;	/* we never return here */
out:
	return ret;
}


int sys_vm86(unsigned long cmd, unsigned long arg, struct pt_regs *regs)
{
	struct kernel_vm86_struct info; /* declare this _on top_,
					 * this avoids wasting of stack space.
					 * This remains on the stack until we
					 * return to 32 bit user space.
					 */
	struct task_struct *tsk;
	int tmp, ret;
	struct vm86plus_struct __user *v86;

	tsk = current;
	switch (cmd) {
	case VM86_REQUEST_IRQ:
	case VM86_FREE_IRQ:
	case VM86_GET_IRQ_BITS:
	case VM86_GET_AND_RESET_IRQ:
		ret = do_vm86_irq_handling(cmd, (int)arg);
		goto out;
	case VM86_PLUS_INSTALL_CHECK:
		/*
		 * NOTE: on old vm86 stuff this will return the error
		 *  from access_ok(), because the subfunction is
		 *  interpreted as (invalid) address to vm86_struct.
		 *  So the installation check works.
		 */
		ret = 0;
		goto out;
	}