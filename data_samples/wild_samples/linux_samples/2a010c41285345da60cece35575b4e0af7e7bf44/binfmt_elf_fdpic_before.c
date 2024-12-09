		if (retval < 0) {
			printk(KERN_ERR "Unable to load interpreter\n");
			goto error;
		}

		allow_write_access(interpreter);
		fput(interpreter);
		interpreter = NULL;
	}

#ifdef CONFIG_MMU
	if (!current->mm->start_brk)
		current->mm->start_brk = current->mm->end_data;

	current->mm->brk = current->mm->start_brk =
		PAGE_ALIGN(current->mm->start_brk);

#else
	/* create a stack area and zero-size brk area */
	stack_size = (stack_size + PAGE_SIZE - 1) & PAGE_MASK;
	if (stack_size < PAGE_SIZE * 2)
		stack_size = PAGE_SIZE * 2;

	stack_prot = PROT_READ | PROT_WRITE;
	if (executable_stack == EXSTACK_ENABLE_X ||
	    (executable_stack == EXSTACK_DEFAULT && VM_STACK_FLAGS & VM_EXEC))
		stack_prot |= PROT_EXEC;

	current->mm->start_brk = vm_mmap(NULL, 0, stack_size, stack_prot,
					 MAP_PRIVATE | MAP_ANONYMOUS |
					 MAP_UNINITIALIZED | MAP_GROWSDOWN,
					 0);

	if (IS_ERR_VALUE(current->mm->start_brk)) {
		retval = current->mm->start_brk;
		current->mm->start_brk = 0;
		goto error;
	}

	current->mm->brk = current->mm->start_brk;
	current->mm->context.end_brk = current->mm->start_brk;
	current->mm->start_stack = current->mm->start_brk + stack_size;
#endif

	retval = create_elf_fdpic_tables(bprm, current->mm, &exec_params,
					 &interp_params);
	if (retval < 0)
		goto error;

	kdebug("- start_code  %lx", current->mm->start_code);
	kdebug("- end_code    %lx", current->mm->end_code);
	kdebug("- start_data  %lx", current->mm->start_data);
	kdebug("- end_data    %lx", current->mm->end_data);
	kdebug("- start_brk   %lx", current->mm->start_brk);
	kdebug("- brk         %lx", current->mm->brk);
	kdebug("- start_stack %lx", current->mm->start_stack);

#ifdef ELF_FDPIC_PLAT_INIT
	/*
	 * The ABI may specify that certain registers be set up in special
	 * ways (on i386 %edx is the address of a DT_FINI function, for
	 * example.  This macro performs whatever initialization to
	 * the regs structure is required.
	 */
	dynaddr = interp_params.dynamic_addr ?: exec_params.dynamic_addr;
	ELF_FDPIC_PLAT_INIT(regs, exec_params.map_addr, interp_params.map_addr,
			    dynaddr);
#endif

	finalize_exec(bprm);
	/* everything is now ready... get the userspace context ready to roll */
	entryaddr = interp_params.entry_addr ?: exec_params.entry_addr;
	start_thread(regs, entryaddr, current->mm->start_stack);

	retval = 0;

error:
	if (interpreter) {
		allow_write_access(interpreter);
		fput(interpreter);
	}
	kfree(interpreter_name);
	kfree(exec_params.phdrs);
	kfree(exec_params.loadmap);
	kfree(interp_params.phdrs);
	kfree(interp_params.loadmap);
	return retval;
}

/*****************************************************************************/

#ifndef ELF_BASE_PLATFORM
/*
 * AT_BASE_PLATFORM indicates the "real" hardware/microarchitecture.
 * If the arch defines ELF_BASE_PLATFORM (in asm/elf.h), the value
 * will be copied to the user stack in the same manner as AT_PLATFORM.
 */
#define ELF_BASE_PLATFORM NULL
#endif

/*
 * present useful information to the program by shovelling it onto the new
 * process's stack
 */
static int create_elf_fdpic_tables(struct linux_binprm *bprm,
				   struct mm_struct *mm,
				   struct elf_fdpic_params *exec_params,
				   struct elf_fdpic_params *interp_params)
{
	const struct cred *cred = current_cred();
	unsigned long sp, csp, nitems;
	elf_caddr_t __user *argv, *envp;
	size_t platform_len = 0, len;
	char *k_platform, *k_base_platform;
	char __user *u_platform, *u_base_platform, *p;
	int loop;
	unsigned long flags = 0;
	int ei_index;
	elf_addr_t *elf_info;

#ifdef CONFIG_MMU
	/* In some cases (e.g. Hyper-Threading), we want to avoid L1 evictions
	 * by the processes running on the same package. One thing we can do is
	 * to shuffle the initial stack for them, so we give the architecture
	 * an opportunity to do so here.
	 */
	sp = arch_align_stack(bprm->p);
#else
	sp = mm->start_stack;

	/* stack the program arguments and environment */
	if (transfer_args_to_stack(bprm, &sp) < 0)
		return -EFAULT;
	sp &= ~15;
#endif

	/*
	 * If this architecture has a platform capability string, copy it
	 * to userspace.  In some cases (Sparc), this info is impossible
	 * for userspace to get any other way, in others (i386) it is
	 * merely difficult.
	 */
	k_platform = ELF_PLATFORM;
	u_platform = NULL;

	if (k_platform) {
		platform_len = strlen(k_platform) + 1;
		sp -= platform_len;
		u_platform = (char __user *) sp;
		if (copy_to_user(u_platform, k_platform, platform_len) != 0)
			return -EFAULT;
	}

	/*
	 * If this architecture has a "base" platform capability
	 * string, copy it to userspace.
	 */
	k_base_platform = ELF_BASE_PLATFORM;
	u_base_platform = NULL;

	if (k_base_platform) {
		platform_len = strlen(k_base_platform) + 1;
		sp -= platform_len;
		u_base_platform = (char __user *) sp;
		if (copy_to_user(u_base_platform, k_base_platform, platform_len) != 0)
			return -EFAULT;
	}

	sp &= ~7UL;

	/* stack the load map(s) */
	len = sizeof(struct elf_fdpic_loadmap);
	len += sizeof(struct elf_fdpic_loadseg) * exec_params->loadmap->nsegs;
	sp = (sp - len) & ~7UL;
	exec_params->map_addr = sp;

	if (copy_to_user((void __user *) sp, exec_params->loadmap, len) != 0)
		return -EFAULT;

	current->mm->context.exec_fdpic_loadmap = (unsigned long) sp;

	if (interp_params->loadmap) {
		len = sizeof(struct elf_fdpic_loadmap);
		len += sizeof(struct elf_fdpic_loadseg) *
			interp_params->loadmap->nsegs;
		sp = (sp - len) & ~7UL;
		interp_params->map_addr = sp;

		if (copy_to_user((void __user *) sp, interp_params->loadmap,
				 len) != 0)
			return -EFAULT;

		current->mm->context.interp_fdpic_loadmap = (unsigned long) sp;
	}

	/* force 16 byte _final_ alignment here for generality */
#define DLINFO_ITEMS 15

	nitems = 1 + DLINFO_ITEMS + (k_platform ? 1 : 0) +
		(k_base_platform ? 1 : 0) + AT_VECTOR_SIZE_ARCH;

	if (bprm->have_execfd)
		nitems++;

	csp = sp;
	sp -= nitems * 2 * sizeof(unsigned long);
	sp -= (bprm->envc + 1) * sizeof(char *);	/* envv[] */
	sp -= (bprm->argc + 1) * sizeof(char *);	/* argv[] */
	sp -= 1 * sizeof(unsigned long);		/* argc */

	csp -= sp & 15UL;
	sp -= sp & 15UL;

	/* Create the ELF interpreter info */
	elf_info = (elf_addr_t *)mm->saved_auxv;
	/* update AT_VECTOR_SIZE_BASE if the number of NEW_AUX_ENT() changes */
#define NEW_AUX_ENT(id, val) \
	do { \
		*elf_info++ = id; \
		*elf_info++ = val; \
	} while (0)

#ifdef ARCH_DLINFO
	/*
	 * ARCH_DLINFO must come first so PPC can do its special alignment of
	 * AUXV.
	 * update AT_VECTOR_SIZE_ARCH if the number of NEW_AUX_ENT() in
	 * ARCH_DLINFO changes
	 */
	ARCH_DLINFO;
#endif
	NEW_AUX_ENT(AT_HWCAP,	ELF_HWCAP);
#ifdef ELF_HWCAP2
	NEW_AUX_ENT(AT_HWCAP2,	ELF_HWCAP2);
#endif
	NEW_AUX_ENT(AT_PAGESZ,	PAGE_SIZE);
	NEW_AUX_ENT(AT_CLKTCK,	CLOCKS_PER_SEC);
	NEW_AUX_ENT(AT_PHDR,	exec_params->ph_addr);
	NEW_AUX_ENT(AT_PHENT,	sizeof(struct elf_phdr));
	NEW_AUX_ENT(AT_PHNUM,	exec_params->hdr.e_phnum);
	NEW_AUX_ENT(AT_BASE,	interp_params->elfhdr_addr);
	if (bprm->interp_flags & BINPRM_FLAGS_PRESERVE_ARGV0)
		flags |= AT_FLAGS_PRESERVE_ARGV0;
	NEW_AUX_ENT(AT_FLAGS,	flags);
	NEW_AUX_ENT(AT_ENTRY,	exec_params->entry_addr);
	NEW_AUX_ENT(AT_UID,	(elf_addr_t) from_kuid_munged(cred->user_ns, cred->uid));
	NEW_AUX_ENT(AT_EUID,	(elf_addr_t) from_kuid_munged(cred->user_ns, cred->euid));
	NEW_AUX_ENT(AT_GID,	(elf_addr_t) from_kgid_munged(cred->user_ns, cred->gid));
	NEW_AUX_ENT(AT_EGID,	(elf_addr_t) from_kgid_munged(cred->user_ns, cred->egid));
	NEW_AUX_ENT(AT_SECURE,	bprm->secureexec);
	NEW_AUX_ENT(AT_EXECFN,	bprm->exec);
	if (k_platform)
		NEW_AUX_ENT(AT_PLATFORM,
			    (elf_addr_t)(unsigned long)u_platform);
	if (k_base_platform)
		NEW_AUX_ENT(AT_BASE_PLATFORM,
			    (elf_addr_t)(unsigned long)u_base_platform);
	if (bprm->have_execfd)
		NEW_AUX_ENT(AT_EXECFD, bprm->execfd);
#undef NEW_AUX_ENT
	/* AT_NULL is zero; clear the rest too */
	memset(elf_info, 0, (char *)mm->saved_auxv +
	       sizeof(mm->saved_auxv) - (char *)elf_info);

	/* And advance past the AT_NULL entry.  */
	elf_info += 2;

	ei_index = elf_info - (elf_addr_t *)mm->saved_auxv;
	csp -= ei_index * sizeof(elf_addr_t);

	/* Put the elf_info on the stack in the right place.  */
	if (copy_to_user((void __user *)csp, mm->saved_auxv,
			 ei_index * sizeof(elf_addr_t)))
		return -EFAULT;

	/* allocate room for argv[] and envv[] */
	csp -= (bprm->envc + 1) * sizeof(elf_caddr_t);
	envp = (elf_caddr_t __user *) csp;
	csp -= (bprm->argc + 1) * sizeof(elf_caddr_t);
	argv = (elf_caddr_t __user *) csp;

	/* stack argc */
	csp -= sizeof(unsigned long);
	if (put_user(bprm->argc, (unsigned long __user *) csp))
		return -EFAULT;

	BUG_ON(csp != sp);

	/* fill in the argv[] array */
#ifdef CONFIG_MMU
	current->mm->arg_start = bprm->p;
#else
	current->mm->arg_start = current->mm->start_stack -
		(MAX_ARG_PAGES * PAGE_SIZE - bprm->p);
#endif

	p = (char __user *) current->mm->arg_start;
	for (loop = bprm->argc; loop > 0; loop--) {
		if (put_user((elf_caddr_t) p, argv++))
			return -EFAULT;
		len = strnlen_user(p, MAX_ARG_STRLEN);
		if (!len || len > MAX_ARG_STRLEN)
			return -EINVAL;
		p += len;
	}
	if (put_user(NULL, argv))
		return -EFAULT;
	current->mm->arg_end = (unsigned long) p;

	/* fill in the envv[] array */
	current->mm->env_start = (unsigned long) p;
	for (loop = bprm->envc; loop > 0; loop--) {
		if (put_user((elf_caddr_t)(unsigned long) p, envp++))
			return -EFAULT;
		len = strnlen_user(p, MAX_ARG_STRLEN);
		if (!len || len > MAX_ARG_STRLEN)
			return -EINVAL;
		p += len;
	}
	if (put_user(NULL, envp))
		return -EFAULT;
	current->mm->env_end = (unsigned long) p;

	mm->start_stack = (unsigned long) sp;
	return 0;
}

/*****************************************************************************/
/*
 * load the appropriate binary image (executable or interpreter) into memory
 * - we assume no MMU is available
 * - if no other PIC bits are set in params->hdr->e_flags
 *   - we assume that the LOADable segments in the binary are independently relocatable
 *   - we assume R/O executable segments are shareable
 * - else
 *   - we assume the loadable parts of the image to require fixed displacement
 *   - the image is not shareable
 */
static int elf_fdpic_map_file(struct elf_fdpic_params *params,
			      struct file *file,
			      struct mm_struct *mm,
			      const char *what)
{
	struct elf_fdpic_loadmap *loadmap;
#ifdef CONFIG_MMU
	struct elf_fdpic_loadseg *mseg;
	unsigned long load_addr;
#endif
	struct elf_fdpic_loadseg *seg;
	struct elf_phdr *phdr;
	unsigned nloads, tmp;
	unsigned long stop;
	int loop, ret;

	/* allocate a load map table */
	nloads = 0;
	for (loop = 0; loop < params->hdr.e_phnum; loop++)
		if (params->phdrs[loop].p_type == PT_LOAD)
			nloads++;

	if (nloads == 0)
		return -ELIBBAD;

	loadmap = kzalloc(struct_size(loadmap, segs, nloads), GFP_KERNEL);
	if (!loadmap)
		return -ENOMEM;

	params->loadmap = loadmap;

	loadmap->version = ELF_FDPIC_LOADMAP_VERSION;
	loadmap->nsegs = nloads;

	/* map the requested LOADs into the memory space */
	switch (params->flags & ELF_FDPIC_FLAG_ARRANGEMENT) {
	case ELF_FDPIC_FLAG_CONSTDISP:
	case ELF_FDPIC_FLAG_CONTIGUOUS:
#ifndef CONFIG_MMU
		ret = elf_fdpic_map_file_constdisp_on_uclinux(params, file, mm);
		if (ret < 0)
			return ret;
		break;
#endif
	default:
		ret = elf_fdpic_map_file_by_direct_mmap(params, file, mm);
		if (ret < 0)
			return ret;
		break;
	}

	/* map the entry point */
	if (params->hdr.e_entry) {
		seg = loadmap->segs;
		for (loop = loadmap->nsegs; loop > 0; loop--, seg++) {
			if (params->hdr.e_entry >= seg->p_vaddr &&
			    params->hdr.e_entry < seg->p_vaddr + seg->p_memsz) {
				params->entry_addr =
					(params->hdr.e_entry - seg->p_vaddr) +
					seg->addr;
				break;
			}
	if (IS_ERR_VALUE(current->mm->start_brk)) {
		retval = current->mm->start_brk;
		current->mm->start_brk = 0;
		goto error;
	}

	current->mm->brk = current->mm->start_brk;
	current->mm->context.end_brk = current->mm->start_brk;
	current->mm->start_stack = current->mm->start_brk + stack_size;
#endif

	retval = create_elf_fdpic_tables(bprm, current->mm, &exec_params,
					 &interp_params);
	if (retval < 0)
		goto error;

	kdebug("- start_code  %lx", current->mm->start_code);
	kdebug("- end_code    %lx", current->mm->end_code);
	kdebug("- start_data  %lx", current->mm->start_data);
	kdebug("- end_data    %lx", current->mm->end_data);
	kdebug("- start_brk   %lx", current->mm->start_brk);
	kdebug("- brk         %lx", current->mm->brk);
	kdebug("- start_stack %lx", current->mm->start_stack);

#ifdef ELF_FDPIC_PLAT_INIT
	/*
	 * The ABI may specify that certain registers be set up in special
	 * ways (on i386 %edx is the address of a DT_FINI function, for
	 * example.  This macro performs whatever initialization to
	 * the regs structure is required.
	 */
	dynaddr = interp_params.dynamic_addr ?: exec_params.dynamic_addr;
	ELF_FDPIC_PLAT_INIT(regs, exec_params.map_addr, interp_params.map_addr,
			    dynaddr);
#endif

	finalize_exec(bprm);
	/* everything is now ready... get the userspace context ready to roll */
	entryaddr = interp_params.entry_addr ?: exec_params.entry_addr;
	start_thread(regs, entryaddr, current->mm->start_stack);

	retval = 0;

error:
	if (interpreter) {
		allow_write_access(interpreter);
		fput(interpreter);
	}