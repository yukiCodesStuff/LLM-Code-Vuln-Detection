	bprm->cred->euid = current_euid();
	bprm->cred->egid = current_egid();

	if (bprm->file->f_path.mnt->mnt_flags & MNT_NOSUID)
		return;

	if (task_no_new_privs(current))
		return;