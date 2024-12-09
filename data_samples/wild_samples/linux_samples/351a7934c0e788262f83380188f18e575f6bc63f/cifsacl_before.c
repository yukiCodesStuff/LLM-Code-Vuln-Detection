			if (rc) {
				cifs_dbg(FYI, "%s: Mapping error %d for group id %d\n",
					 __func__, rc, id);
				kfree(ngroup_sid_ptr);
				return rc;
			}
			cifs_copy_sid(group_sid_ptr, ngroup_sid_ptr);
			kfree(ngroup_sid_ptr);
			*aclflag = CIFS_ACL_GROUP;
		}
	}

	return rc;
}

static struct cifs_ntsd *get_cifs_acl_by_fid(struct cifs_sb_info *cifs_sb,
		__u16 fid, u32 *pacllen)
{
	struct cifs_ntsd *pntsd = NULL;
	unsigned int xid;
	int rc;
	struct tcon_link *tlink = cifs_sb_tlink(cifs_sb);

	if (IS_ERR(tlink))
		return ERR_CAST(tlink);

	xid = get_xid();
	rc = CIFSSMBGetCIFSACL(xid, tlink_tcon(tlink), fid, &pntsd, pacllen);
	free_xid(xid);

	cifs_put_tlink(tlink);

	cifs_dbg(FYI, "%s: rc = %d ACL len %d\n", __func__, rc, *pacllen);
	if (rc)
		return ERR_PTR(rc);
	return pntsd;
}

static struct cifs_ntsd *get_cifs_acl_by_path(struct cifs_sb_info *cifs_sb,
		const char *path, u32 *pacllen)
{
	struct cifs_ntsd *pntsd = NULL;
	int oplock = 0;
	unsigned int xid;
	int rc, create_options = 0;
	struct cifs_tcon *tcon;
	struct tcon_link *tlink = cifs_sb_tlink(cifs_sb);
	struct cifs_fid fid;
	struct cifs_open_parms oparms;

	if (IS_ERR(tlink))
		return ERR_CAST(tlink);

	tcon = tlink_tcon(tlink);
	xid = get_xid();

	if (backup_cred(cifs_sb))
		create_options |= CREATE_OPEN_BACKUP_INTENT;

	oparms.tcon = tcon;
	oparms.cifs_sb = cifs_sb;
	oparms.desired_access = READ_CONTROL;
	oparms.create_options = create_options;
	oparms.disposition = FILE_OPEN;
	oparms.path = path;
	oparms.fid = &fid;
	oparms.reconnect = false;

	rc = CIFS_open(xid, &oparms, &oplock, NULL);
	if (!rc) {
		rc = CIFSSMBGetCIFSACL(xid, tcon, fid.netfid, &pntsd, pacllen);
		CIFSSMBClose(xid, tcon, fid.netfid);
	}
{
	struct cifs_ntsd *pntsd = NULL;
	unsigned int xid;
	int rc;
	struct tcon_link *tlink = cifs_sb_tlink(cifs_sb);

	if (IS_ERR(tlink))
		return ERR_CAST(tlink);

	xid = get_xid();
	rc = CIFSSMBGetCIFSACL(xid, tlink_tcon(tlink), fid, &pntsd, pacllen);
	free_xid(xid);

	cifs_put_tlink(tlink);

	cifs_dbg(FYI, "%s: rc = %d ACL len %d\n", __func__, rc, *pacllen);
	if (rc)
		return ERR_PTR(rc);
	return pntsd;
}

static struct cifs_ntsd *get_cifs_acl_by_path(struct cifs_sb_info *cifs_sb,
		const char *path, u32 *pacllen)
{
	struct cifs_ntsd *pntsd = NULL;
	int oplock = 0;
	unsigned int xid;
	int rc, create_options = 0;
	struct cifs_tcon *tcon;
	struct tcon_link *tlink = cifs_sb_tlink(cifs_sb);
	struct cifs_fid fid;
	struct cifs_open_parms oparms;

	if (IS_ERR(tlink))
		return ERR_CAST(tlink);

	tcon = tlink_tcon(tlink);
	xid = get_xid();

	if (backup_cred(cifs_sb))
		create_options |= CREATE_OPEN_BACKUP_INTENT;

	oparms.tcon = tcon;
	oparms.cifs_sb = cifs_sb;
	oparms.desired_access = READ_CONTROL;
	oparms.create_options = create_options;
	oparms.disposition = FILE_OPEN;
	oparms.path = path;
	oparms.fid = &fid;
	oparms.reconnect = false;

	rc = CIFS_open(xid, &oparms, &oplock, NULL);
	if (!rc) {
		rc = CIFSSMBGetCIFSACL(xid, tcon, fid.netfid, &pntsd, pacllen);
		CIFSSMBClose(xid, tcon, fid.netfid);
	}
{
	struct cifs_ntsd *pntsd = NULL;
	struct cifsFileInfo *open_file = NULL;

	if (inode)
		open_file = find_readable_file(CIFS_I(inode), true);
	if (!open_file)
		return get_cifs_acl_by_path(cifs_sb, path, pacllen);

	pntsd = get_cifs_acl_by_fid(cifs_sb, open_file->fid.netfid, pacllen);
	cifsFileInfo_put(open_file);
	return pntsd;
}

 /* Set an ACL on the server */
int set_cifs_acl(struct cifs_ntsd *pnntsd, __u32 acllen,
			struct inode *inode, const char *path, int aclflag)
{
	int oplock = 0;
	unsigned int xid;
	int rc, access_flags, create_options = 0;
	struct cifs_tcon *tcon;
	struct cifs_sb_info *cifs_sb = CIFS_SB(inode->i_sb);
	struct tcon_link *tlink = cifs_sb_tlink(cifs_sb);
	struct cifs_fid fid;
	struct cifs_open_parms oparms;

	if (IS_ERR(tlink))
		return PTR_ERR(tlink);

	tcon = tlink_tcon(tlink);
	xid = get_xid();

	if (backup_cred(cifs_sb))
		create_options |= CREATE_OPEN_BACKUP_INTENT;

	if (aclflag == CIFS_ACL_OWNER || aclflag == CIFS_ACL_GROUP)
		access_flags = WRITE_OWNER;
	else
		access_flags = WRITE_DAC;

	oparms.tcon = tcon;
	oparms.cifs_sb = cifs_sb;
	oparms.desired_access = access_flags;
	oparms.create_options = create_options;
	oparms.disposition = FILE_OPEN;
	oparms.path = path;
	oparms.fid = &fid;
	oparms.reconnect = false;

	rc = CIFS_open(xid, &oparms, &oplock, NULL);
	if (rc) {
		cifs_dbg(VFS, "Unable to open file to set ACL\n");
		goto out;
	}
	if (rc) {
		cifs_dbg(VFS, "Unable to open file to set ACL\n");
		goto out;
	}

	rc = CIFSSMBSetCIFSACL(xid, tcon, fid.netfid, pnntsd, acllen, aclflag);
	cifs_dbg(NOISY, "SetCIFSACL rc = %d\n", rc);

	CIFSSMBClose(xid, tcon, fid.netfid);
out:
	free_xid(xid);
	cifs_put_tlink(tlink);
	return rc;
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
	} else {
		rc = parse_sec_desc(cifs_sb, pntsd, acllen, fattr);
		kfree(pntsd);
		if (rc)
			cifs_dbg(VFS, "parse sec desc failed rc = %d\n", rc);
	}

	return rc;
}

/* Convert mode bits to an ACL so we can update the ACL on the server */
int
id_mode_to_cifs_acl(struct inode *inode, const char *path, __u64 nmode,
			kuid_t uid, kgid_t gid)
{
	int rc = 0;
	int aclflag = CIFS_ACL_DACL; /* default flag to set */
	__u32 secdesclen = 0;
	struct cifs_ntsd *pntsd = NULL; /* acl obtained from server */
	struct cifs_ntsd *pnntsd = NULL; /* modified acl to be sent to server */
	struct cifs_sb_info *cifs_sb = CIFS_SB(inode->i_sb);
	struct tcon_link *tlink = cifs_sb_tlink(cifs_sb);
	struct cifs_tcon *tcon;

	if (IS_ERR(tlink))
		return PTR_ERR(tlink);
	tcon = tlink_tcon(tlink);

	cifs_dbg(NOISY, "set ACL from mode for %s\n", path);

	/* Get the security descriptor */

	if (tcon->ses->server->ops->get_acl == NULL) {
		cifs_put_tlink(tlink);
		return -EOPNOTSUPP;
	}

	pntsd = tcon->ses->server->ops->get_acl(cifs_sb, inode, path,
						&secdesclen);
	if (IS_ERR(pntsd)) {
		rc = PTR_ERR(pntsd);
		cifs_dbg(VFS, "%s: error %d getting sec desc\n", __func__, rc);
		cifs_put_tlink(tlink);
		return rc;
	}

	/*
	 * Add three ACEs for owner, group, everyone getting rid of other ACEs
	 * as chmod disables ACEs and set the security descriptor. Allocate
	 * memory for the smb header, set security descriptor request security
	 * descriptor parameters, and secuirty descriptor itself
	 */
	secdesclen = max_t(u32, secdesclen, DEFAULT_SEC_DESC_LEN);
	pnntsd = kmalloc(secdesclen, GFP_KERNEL);
	if (!pnntsd) {
		kfree(pntsd);
		cifs_put_tlink(tlink);
		return -ENOMEM;
	}

	rc = build_sec_desc(pntsd, pnntsd, secdesclen, nmode, uid, gid,
				&aclflag);

	cifs_dbg(NOISY, "build_sec_desc rc: %d\n", rc);

	if (tcon->ses->server->ops->set_acl == NULL)
		rc = -EOPNOTSUPP;

	if (!rc) {
		/* Set the security descriptor */
		rc = tcon->ses->server->ops->set_acl(pnntsd, secdesclen, inode,
						     path, aclflag);
		cifs_dbg(NOISY, "set_cifs_acl rc: %d\n", rc);
	}
	cifs_put_tlink(tlink);

	kfree(pnntsd);
	kfree(pntsd);
	return rc;
}
	} else {
		rc = parse_sec_desc(cifs_sb, pntsd, acllen, fattr);
		kfree(pntsd);
		if (rc)
			cifs_dbg(VFS, "parse sec desc failed rc = %d\n", rc);
	}

	return rc;
}

/* Convert mode bits to an ACL so we can update the ACL on the server */
int
id_mode_to_cifs_acl(struct inode *inode, const char *path, __u64 nmode,
			kuid_t uid, kgid_t gid)
{
	int rc = 0;
	int aclflag = CIFS_ACL_DACL; /* default flag to set */
	__u32 secdesclen = 0;
	struct cifs_ntsd *pntsd = NULL; /* acl obtained from server */
	struct cifs_ntsd *pnntsd = NULL; /* modified acl to be sent to server */
	struct cifs_sb_info *cifs_sb = CIFS_SB(inode->i_sb);
	struct tcon_link *tlink = cifs_sb_tlink(cifs_sb);
	struct cifs_tcon *tcon;

	if (IS_ERR(tlink))
		return PTR_ERR(tlink);
	tcon = tlink_tcon(tlink);

	cifs_dbg(NOISY, "set ACL from mode for %s\n", path);

	/* Get the security descriptor */

	if (tcon->ses->server->ops->get_acl == NULL) {
		cifs_put_tlink(tlink);
		return -EOPNOTSUPP;
	}

	pntsd = tcon->ses->server->ops->get_acl(cifs_sb, inode, path,
						&secdesclen);
	if (IS_ERR(pntsd)) {
		rc = PTR_ERR(pntsd);
		cifs_dbg(VFS, "%s: error %d getting sec desc\n", __func__, rc);
		cifs_put_tlink(tlink);
		return rc;
	}

	/*
	 * Add three ACEs for owner, group, everyone getting rid of other ACEs
	 * as chmod disables ACEs and set the security descriptor. Allocate
	 * memory for the smb header, set security descriptor request security
	 * descriptor parameters, and secuirty descriptor itself
	 */
	secdesclen = max_t(u32, secdesclen, DEFAULT_SEC_DESC_LEN);
	pnntsd = kmalloc(secdesclen, GFP_KERNEL);
	if (!pnntsd) {
		kfree(pntsd);
		cifs_put_tlink(tlink);
		return -ENOMEM;
	}

	rc = build_sec_desc(pntsd, pnntsd, secdesclen, nmode, uid, gid,
				&aclflag);

	cifs_dbg(NOISY, "build_sec_desc rc: %d\n", rc);

	if (tcon->ses->server->ops->set_acl == NULL)
		rc = -EOPNOTSUPP;

	if (!rc) {
		/* Set the security descriptor */
		rc = tcon->ses->server->ops->set_acl(pnntsd, secdesclen, inode,
						     path, aclflag);
		cifs_dbg(NOISY, "set_cifs_acl rc: %d\n", rc);
	}
	cifs_put_tlink(tlink);

	kfree(pnntsd);
	kfree(pntsd);
	return rc;
}