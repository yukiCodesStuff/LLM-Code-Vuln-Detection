{
	int nnp = (bprm->unsafe & LSM_UNSAFE_NO_NEW_PRIVS);
	int nosuid = (bprm->file->f_path.mnt->mnt_flags & MNT_NOSUID);
	int rc;

	if (!nnp && !nosuid)
		return 0; /* neither NNP nor nosuid */

	if (new_tsec->sid == old_tsec->sid)
		return 0; /* No change in credentials */

	/*
	 * The only transitions we permit under NNP or nosuid
	 * are transitions to bounded SIDs, i.e. SIDs that are
	 * guaranteed to only be allowed a subset of the permissions
	 * of the current SID.
	 */
	rc = security_bounded_transition(old_tsec->sid, new_tsec->sid);
	if (rc) {
		/*
		 * On failure, preserve the errno values for NNP vs nosuid.
		 * NNP:  Operation not permitted for caller.
		 * nosuid:  Permission denied to file.
		 */
		if (nnp)
			return -EPERM;
		else
			return -EACCES;
	}
	return 0;
}