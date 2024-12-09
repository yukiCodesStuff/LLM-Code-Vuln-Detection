	  _IOC(_IOC_DIR(cmd), _IOC_TYPE(cmd), _IOC_NR(cmd), sizeof(type))

#ifdef BROKEN_X86_ALIGNMENT
STATIC int
xfs_compat_flock64_copyin(
	xfs_flock64_t		*bf,
	compat_xfs_flock64_t	__user *arg32)
{
	if (get_user(bf->l_type,	&arg32->l_type) ||
	    get_user(bf->l_whence,	&arg32->l_whence) ||
	    get_user(bf->l_start,	&arg32->l_start) ||
	    get_user(bf->l_len,		&arg32->l_len) ||
	    get_user(bf->l_sysid,	&arg32->l_sysid) ||
	    get_user(bf->l_pid,		&arg32->l_pid) ||
	    copy_from_user(bf->l_pad,	&arg32->l_pad,	4*sizeof(u32)))
		return -EFAULT;
	return 0;
}

STATIC int
xfs_compat_ioc_fsgeometry_v1(
	struct xfs_mount	  *mp,
	compat_xfs_fsop_geom_v1_t __user *arg32)

	switch (cmd) {
#if defined(BROKEN_X86_ALIGNMENT)
	case XFS_IOC_ALLOCSP_32:
	case XFS_IOC_FREESP_32:
	case XFS_IOC_ALLOCSP64_32:
	case XFS_IOC_FREESP64_32: {
		struct xfs_flock64	bf;

		if (xfs_compat_flock64_copyin(&bf, arg))
			return -EFAULT;
		cmd = _NATIVE_IOC(cmd, struct xfs_flock64);
		return xfs_ioc_space(filp, &bf);
	}
	case XFS_IOC_FSGEOMETRY_V1_32:
		return xfs_compat_ioc_fsgeometry_v1(ip->i_mount, arg);
	case XFS_IOC_FSGROWFSDATA_32: {
		struct xfs_growfs_data	in;