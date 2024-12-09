	bprm->cred->euid = current_euid();
	bprm->cred->egid = current_egid();

	if (!mnt_may_suid(bprm->file->f_path.mnt))
		return;

	if (task_no_new_privs(current))
		return;