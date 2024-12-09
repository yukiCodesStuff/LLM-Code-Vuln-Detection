	if (!file_caps_enabled)
		return 0;

	if (bprm->file->f_path.mnt->mnt_flags & MNT_NOSUID)
		return 0;
	if (!current_in_userns(bprm->file->f_path.mnt->mnt_sb->s_user_ns))
		return 0;

	rc = get_vfs_caps_from_disk(bprm->file->f_path.dentry, &vcaps);