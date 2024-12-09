	 * the stack. They aren't stored until much later when we can't
	 * signal to the parent that the child has run out of stack space.
	 * Instead, calculate it here so it's possible to fail gracefully.
	 *
	 * In the case of argc = 0, make sure there is space for adding a
	 * empty string (which will bump argc to 1), to ensure confused
	 * userspace programs don't start processing from argv[1], thinking
	 * argc can never be 0, to keep them from walking envp by accident.
	 * See do_execveat_common().
	 */
	ptr_size = (max(bprm->argc, 1) + bprm->envc) * sizeof(void *);
	if (limit <= ptr_size)
		return -E2BIG;
	limit -= ptr_size;

	}

	retval = count(argv, MAX_ARG_STRINGS);
	if (retval == 0)
		pr_warn_once("process '%s' launched '%s' with NULL argv: empty string added\n",
			     current->comm, bprm->filename);
	if (retval < 0)
		goto out_free;
	bprm->argc = retval;

	if (retval < 0)
		goto out_free;

	/*
	 * When argv is empty, add an empty string ("") as argv[0] to
	 * ensure confused userspace programs that start processing
	 * from argv[1] won't end up walking envp. See also
	 * bprm_stack_limits().
	 */
	if (bprm->argc == 0) {
		retval = copy_string_kernel("", bprm);
		if (retval < 0)
			goto out_free;
		bprm->argc = 1;
	}

	retval = bprm_execve(bprm, fd, filename, flags);
out_free:
	free_bprm(bprm);

	}

	retval = count_strings_kernel(argv);
	if (WARN_ON_ONCE(retval == 0))
		retval = -EINVAL;
	if (retval < 0)
		goto out_free;
	bprm->argc = retval;
