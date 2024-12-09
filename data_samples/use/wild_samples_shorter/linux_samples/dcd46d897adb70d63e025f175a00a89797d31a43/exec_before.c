	 * the stack. They aren't stored until much later when we can't
	 * signal to the parent that the child has run out of stack space.
	 * Instead, calculate it here so it's possible to fail gracefully.
	 */
	ptr_size = (bprm->argc + bprm->envc) * sizeof(void *);
	if (limit <= ptr_size)
		return -E2BIG;
	limit -= ptr_size;

	}

	retval = count(argv, MAX_ARG_STRINGS);
	if (retval < 0)
		goto out_free;
	bprm->argc = retval;

	if (retval < 0)
		goto out_free;

	retval = bprm_execve(bprm, fd, filename, flags);
out_free:
	free_bprm(bprm);

	}

	retval = count_strings_kernel(argv);
	if (retval < 0)
		goto out_free;
	bprm->argc = retval;
