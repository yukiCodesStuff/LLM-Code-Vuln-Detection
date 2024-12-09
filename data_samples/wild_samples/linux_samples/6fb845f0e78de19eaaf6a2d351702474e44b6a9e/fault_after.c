			      sizeof(struct ldttss_desc))) {
		pr_alert("%s: 0x%hx -- GDT entry is not readable\n",
			 name, index);
		return;
	}

	addr = desc.base0 | (desc.base1 << 16) | ((unsigned long)desc.base2 << 24);
#ifdef CONFIG_X86_64
	addr |= ((u64)desc.base3 << 32);
#endif
	pr_alert("%s: 0x%hx -- base=0x%lx limit=0x%x\n",
		 name, index, addr, (desc.limit0 | (desc.limit1 << 16)));
}

/*
 * This helper function transforms the #PF error_code bits into
 * "[PROT] [USER]" type of descriptive, almost human-readable error strings:
 */
static void err_str_append(unsigned long error_code, char *buf, unsigned long mask, const char *txt)
{
	if (error_code & mask) {
		if (buf[0])
			strcat(buf, " ");
		strcat(buf, txt);
	}
}

static void
show_fault_oops(struct pt_regs *regs, unsigned long error_code, unsigned long address)
{
	char err_txt[64];

	if (!oops_may_print())
		return;

	if (error_code & X86_PF_INSTR) {
		unsigned int level;
		pgd_t *pgd;
		pte_t *pte;

		pgd = __va(read_cr3_pa());
		pgd += pgd_index(address);

		pte = lookup_address_in_pgd(pgd, address, &level);

		if (pte && pte_present(*pte) && !pte_exec(*pte))
			pr_crit("kernel tried to execute NX-protected page - exploit attempt? (uid: %d)\n",
				from_kuid(&init_user_ns, current_uid()));
		if (pte && pte_present(*pte) && pte_exec(*pte) &&
				(pgd_flags(*pgd) & _PAGE_USER) &&
				(__read_cr4() & X86_CR4_SMEP))
			pr_crit("unable to execute userspace code (SMEP?) (uid: %d)\n",
				from_kuid(&init_user_ns, current_uid()));
	}

	pr_alert("BUG: unable to handle kernel %s at %px\n",
		 address < PAGE_SIZE ? "NULL pointer dereference" : "paging request",
		 (void *)address);

	err_txt[0] = 0;

	/*
	 * Note: length of these appended strings including the separation space and the
	 * zero delimiter must fit into err_txt[].
	 */
	err_str_append(error_code, err_txt, X86_PF_PROT,  "[PROT]" );
	err_str_append(error_code, err_txt, X86_PF_WRITE, "[WRITE]");
	err_str_append(error_code, err_txt, X86_PF_USER,  "[USER]" );
	err_str_append(error_code, err_txt, X86_PF_RSVD,  "[RSVD]" );
	err_str_append(error_code, err_txt, X86_PF_INSTR, "[INSTR]");
	err_str_append(error_code, err_txt, X86_PF_PK,    "[PK]"   );

	pr_alert("#PF error: %s\n", error_code ? err_txt : "[normal kernel read fault]");

	if (!(error_code & X86_PF_USER) && user_mode(regs)) {
		struct desc_ptr idt, gdt;
		u16 ldtr, tr;

		pr_alert("This was a system access from user code\n");

		/*
		 * This can happen for quite a few reasons.  The more obvious
		 * ones are faults accessing the GDT, or LDT.  Perhaps
		 * surprisingly, if the CPU tries to deliver a benign or
		 * contributory exception from user code and gets a page fault
		 * during delivery, the page fault can be delivered as though
		 * it originated directly from user code.  This could happen
		 * due to wrong permissions on the IDT, GDT, LDT, TSS, or
		 * kernel or IST stack.
		 */
		store_idt(&idt);

		/* Usable even on Xen PV -- it's just slow. */
		native_store_gdt(&gdt);

		pr_alert("IDT: 0x%lx (limit=0x%hx) GDT: 0x%lx (limit=0x%hx)\n",
			 idt.address, idt.size, gdt.address, gdt.size);

		store_ldt(ldtr);
		show_ldttss(&gdt, "LDTR", ldtr);

		store_tr(tr);
		show_ldttss(&gdt, "TR", tr);
	}

	dump_pagetable(address);
}

static noinline void
pgtable_bad(struct pt_regs *regs, unsigned long error_code,
	    unsigned long address)
{
	struct task_struct *tsk;
	unsigned long flags;
	int sig;

	flags = oops_begin();
	tsk = current;
	sig = SIGKILL;

	printk(KERN_ALERT "%s: Corrupted page table at address %lx\n",
	       tsk->comm, address);
	dump_pagetable(address);

	if (__die("Bad pagetable", regs, error_code))
		sig = 0;

	oops_end(flags, regs, sig);
}

static void set_signal_archinfo(unsigned long address,
				unsigned long error_code)
{
	struct task_struct *tsk = current;

	/*
	 * To avoid leaking information about the kernel page
	 * table layout, pretend that user-mode accesses to
	 * kernel addresses are always protection faults.
	 */
	if (address >= TASK_SIZE_MAX)
		error_code |= X86_PF_PROT;

	tsk->thread.trap_nr = X86_TRAP_PF;
	tsk->thread.error_code = error_code | X86_PF_USER;
	tsk->thread.cr2 = address;
}

static noinline void
no_context(struct pt_regs *regs, unsigned long error_code,
	   unsigned long address, int signal, int si_code)
{
	struct task_struct *tsk = current;
	unsigned long flags;
	int sig;

	if (user_mode(regs)) {
		/*
		 * This is an implicit supervisor-mode access from user
		 * mode.  Bypass all the kernel-mode recovery code and just
		 * OOPS.
		 */
		goto oops;
	}

	/* Are we prepared to handle this kernel fault? */
	if (fixup_exception(regs, X86_TRAP_PF, error_code, address)) {
		/*
		 * Any interrupt that takes a fault gets the fixup. This makes
		 * the below recursive fault logic only apply to a faults from
		 * task context.
		 */
		if (in_interrupt())
			return;

		/*
		 * Per the above we're !in_interrupt(), aka. task context.
		 *
		 * In this case we need to make sure we're not recursively
		 * faulting through the emulate_vsyscall() logic.
		 */
		if (current->thread.sig_on_uaccess_err && signal) {
			set_signal_archinfo(address, error_code);

			/* XXX: hwpoison faults will set the wrong code. */
			force_sig_fault(signal, si_code, (void __user *)address,
					tsk);
		}