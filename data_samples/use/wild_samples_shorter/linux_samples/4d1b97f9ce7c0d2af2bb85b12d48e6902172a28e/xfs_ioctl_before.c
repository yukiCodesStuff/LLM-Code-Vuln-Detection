	return error;
}

int
xfs_ioc_space(
	struct file		*filp,
	xfs_flock64_t		*bf)
{
	struct inode		*inode = file_inode(filp);
	struct xfs_inode	*ip = XFS_I(inode);
	struct iattr		iattr;
	enum xfs_prealloc_flags	flags = XFS_PREALLOC_CLEAR;
	uint			iolock = XFS_IOLOCK_EXCL | XFS_MMAPLOCK_EXCL;
	int			error;

	if (inode->i_flags & (S_IMMUTABLE|S_APPEND))
		return -EPERM;

	if (!(filp->f_mode & FMODE_WRITE))
		return -EBADF;

	if (!S_ISREG(inode->i_mode))
		return -EINVAL;

	if (xfs_is_always_cow_inode(ip))
		return -EOPNOTSUPP;

	if (filp->f_flags & O_DSYNC)
		flags |= XFS_PREALLOC_SYNC;
	if (filp->f_mode & FMODE_NOCMTIME)
		flags |= XFS_PREALLOC_INVISIBLE;

	error = mnt_want_write_file(filp);
	if (error)
		return error;

	xfs_ilock(ip, iolock);
	error = xfs_break_layouts(inode, &iolock, BREAK_UNMAP);
	if (error)
		goto out_unlock;
	inode_dio_wait(inode);

	switch (bf->l_whence) {
	case 0: /*SEEK_SET*/
		break;
	case 1: /*SEEK_CUR*/
		bf->l_start += filp->f_pos;
		break;
	case 2: /*SEEK_END*/
		bf->l_start += XFS_ISIZE(ip);
		break;
	default:
		error = -EINVAL;
		goto out_unlock;
	}

	if (bf->l_start < 0 || bf->l_start > inode->i_sb->s_maxbytes) {
		error = -EINVAL;
		goto out_unlock;
	}

	if (bf->l_start > XFS_ISIZE(ip)) {
		error = xfs_alloc_file_space(ip, XFS_ISIZE(ip),
				bf->l_start - XFS_ISIZE(ip), 0);
		if (error)
			goto out_unlock;
	}

	iattr.ia_valid = ATTR_SIZE;
	iattr.ia_size = bf->l_start;
	error = xfs_vn_setattr_size(file_mnt_user_ns(filp), file_dentry(filp),
				    &iattr);
	if (error)
		goto out_unlock;

	error = xfs_update_prealloc_flags(ip, flags);

out_unlock:
	xfs_iunlock(ip, iolock);
	mnt_drop_write_file(filp);
	return error;
}

/* Return 0 on success or positive error */
int
xfs_fsbulkstat_one_fmt(
	struct xfs_ibulk		*breq,
	case XFS_IOC_ALLOCSP:
	case XFS_IOC_FREESP:
	case XFS_IOC_ALLOCSP64:
	case XFS_IOC_FREESP64: {
		xfs_flock64_t		bf;

		if (copy_from_user(&bf, arg, sizeof(bf)))
			return -EFAULT;
		return xfs_ioc_space(filp, &bf);
	}
	case XFS_IOC_DIOINFO: {
		struct xfs_buftarg	*target = xfs_inode_buftarg(ip);
		struct dioattr		da;
