	return error;
}

/* Return 0 on success or positive error */
int
xfs_fsbulkstat_one_fmt(
	struct xfs_ibulk		*breq,
	case XFS_IOC_ALLOCSP:
	case XFS_IOC_FREESP:
	case XFS_IOC_ALLOCSP64:
	case XFS_IOC_FREESP64:
		xfs_warn_once(mp,
	"%s should use fallocate; XFS_IOC_{ALLOC,FREE}SP ioctl unsupported",
				current->comm);
		return -ENOTTY;
	case XFS_IOC_DIOINFO: {
		struct xfs_buftarg	*target = xfs_inode_buftarg(ip);
		struct dioattr		da;
