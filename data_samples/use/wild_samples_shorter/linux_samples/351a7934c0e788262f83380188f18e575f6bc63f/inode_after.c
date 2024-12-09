int
cifs_get_inode_info(struct inode **inode, const char *full_path,
		    FILE_ALL_INFO *data, struct super_block *sb, int xid,
		    const struct cifs_fid *fid)
{
	bool validinum = false;
	__u16 srchflgs;
	int rc = 0, tmprc = ENOSYS;