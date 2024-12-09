
	exe_file = get_mm_exe_file(oldmm);
	RCU_INIT_POINTER(mm->exe_file, exe_file);
	/*
	 * We depend on the oldmm having properly denied write access to the
	 * exe_file already.
	 */
	if (exe_file && deny_write_access(exe_file))
		pr_warn_once("deny_write_access() failed in %s\n", __func__);
}

#ifdef CONFIG_MMU
static __latent_entropy int dup_mmap(struct mm_struct *mm,
	 */
	old_exe_file = rcu_dereference_raw(mm->exe_file);

	if (new_exe_file) {
		/*
		 * We expect the caller (i.e., sys_execve) to already denied
		 * write access, so this is unlikely to fail.
		 */
		if (unlikely(deny_write_access(new_exe_file)))
			return -EACCES;
		get_file(new_exe_file);
	}
	rcu_assign_pointer(mm->exe_file, new_exe_file);
	if (old_exe_file) {
		allow_write_access(old_exe_file);
		fput(old_exe_file);
	}
	return 0;
}

/**
			return ret;
	}

	ret = deny_write_access(new_exe_file);
	if (ret)
		return -EACCES;
	get_file(new_exe_file);

	/* set the new file */
	mmap_write_lock(mm);
	rcu_assign_pointer(mm->exe_file, new_exe_file);
	mmap_write_unlock(mm);

	if (old_exe_file) {
		allow_write_access(old_exe_file);
		fput(old_exe_file);
	}
	return 0;
}

/**