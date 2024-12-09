	if (!file_caps_enabled)
		return 0;

	if (!mnt_may_suid(bprm->file->f_path.mnt))
		return 0;

	/*
	 * This check is redundant with mnt_may_suid() but is kept to make
	 * explicit that capability bits are limited to s_user_ns and its
	 * descendants.
	 */
	if (!current_in_userns(bprm->file->f_path.mnt->mnt_sb->s_user_ns))
		return 0;

	rc = get_vfs_caps_from_disk(bprm->file->f_path.dentry, &vcaps);