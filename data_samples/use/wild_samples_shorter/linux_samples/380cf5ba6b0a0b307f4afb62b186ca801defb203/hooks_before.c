			    const struct task_security_struct *new_tsec)
{
	int nnp = (bprm->unsafe & LSM_UNSAFE_NO_NEW_PRIVS);
	int nosuid = (bprm->file->f_path.mnt->mnt_flags & MNT_NOSUID);
	int rc;

	if (!nnp && !nosuid)
		return 0; /* neither NNP nor nosuid */