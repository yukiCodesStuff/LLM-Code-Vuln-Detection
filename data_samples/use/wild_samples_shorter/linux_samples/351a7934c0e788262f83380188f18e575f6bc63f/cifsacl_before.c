	return rc;
}

static struct cifs_ntsd *get_cifs_acl_by_fid(struct cifs_sb_info *cifs_sb,
		__u16 fid, u32 *pacllen)
{
	struct cifs_ntsd *pntsd = NULL;
	unsigned int xid;
	int rc;
		return ERR_CAST(tlink);

	xid = get_xid();
	rc = CIFSSMBGetCIFSACL(xid, tlink_tcon(tlink), fid, &pntsd, pacllen);
	free_xid(xid);

	cifs_put_tlink(tlink);

	if (!open_file)
		return get_cifs_acl_by_path(cifs_sb, path, pacllen);

	pntsd = get_cifs_acl_by_fid(cifs_sb, open_file->fid.netfid, pacllen);
	cifsFileInfo_put(open_file);
	return pntsd;
}

/* Translate the CIFS ACL (simlar to NTFS ACL) for a file into mode bits */
int
cifs_acl_to_fattr(struct cifs_sb_info *cifs_sb, struct cifs_fattr *fattr,
		  struct inode *inode, const char *path, const __u16 *pfid)
{
	struct cifs_ntsd *pntsd = NULL;
	u32 acllen = 0;
	int rc = 0;

	cifs_dbg(NOISY, "converting ACL to mode for %s\n", path);

	if (pfid)
		pntsd = get_cifs_acl_by_fid(cifs_sb, *pfid, &acllen);
	else
		pntsd = get_cifs_acl(cifs_sb, inode, path, &acllen);

	/* if we can retrieve the ACL, now parse Access Control Entries, ACEs */
	if (IS_ERR(pntsd)) {
		rc = PTR_ERR(pntsd);
		cifs_dbg(VFS, "%s: error %d getting sec desc\n", __func__, rc);
			cifs_dbg(VFS, "parse sec desc failed rc = %d\n", rc);
	}

	return rc;
}

/* Convert mode bits to an ACL so we can update the ACL on the server */