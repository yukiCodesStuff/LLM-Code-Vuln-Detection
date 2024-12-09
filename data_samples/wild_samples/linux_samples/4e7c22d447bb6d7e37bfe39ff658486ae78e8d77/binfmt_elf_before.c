		if (padzero(elf_bss)) {
			error = -EFAULT;
			goto out;
		}

		/* What we have mapped so far */
		elf_bss = ELF_PAGESTART(elf_bss + ELF_MIN_ALIGN - 1);

		/* Map the last of the bss segment */
		error = vm_brk(elf_bss, last_bss - elf_bss);
		if (BAD_ADDR(error))
			goto out;
	}

	error = load_addr;
out:
	return error;
}

/*
 * These are the functions used to load ELF style executables and shared
 * libraries.  There is no binary dependent code anywhere else.
 */

#ifndef STACK_RND_MASK
#define STACK_RND_MASK (0x7ff >> (PAGE_SHIFT - 12))	/* 8MB of VA */
#endif

static unsigned long randomize_stack_top(unsigned long stack_top)
{
	unsigned int random_variable = 0;

	if ((current->flags & PF_RANDOMIZE) &&
		!(current->personality & ADDR_NO_RANDOMIZE)) {
		random_variable = get_random_int() & STACK_RND_MASK;
		random_variable <<= PAGE_SHIFT;
	}
#ifdef CONFIG_STACK_GROWSUP
	return PAGE_ALIGN(stack_top) + random_variable;
#else
	return PAGE_ALIGN(stack_top) - random_variable;
#endif
}