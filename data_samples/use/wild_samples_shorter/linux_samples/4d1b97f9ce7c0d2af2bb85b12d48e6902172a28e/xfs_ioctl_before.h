struct xfs_ibulk;
struct xfs_inogrp;


extern int
xfs_ioc_space(
	struct file		*filp,
	xfs_flock64_t		*bf);

int
xfs_ioc_swapext(
	xfs_swapext_t	*sxp);
