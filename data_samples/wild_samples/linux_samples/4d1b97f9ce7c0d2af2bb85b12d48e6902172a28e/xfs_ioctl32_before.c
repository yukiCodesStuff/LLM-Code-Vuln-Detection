// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2004-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#include <linux/mount.h>
#include <linux/fsmap.h>
#include "xfs.h"
#include "xfs_fs.h"
#include "xfs_shared.h"
#include "xfs_format.h"
#include "xfs_log_format.h"
#include "xfs_trans_resv.h"
#include "xfs_mount.h"
#include "xfs_inode.h"
#include "xfs_iwalk.h"
#include "xfs_itable.h"
#include "xfs_fsops.h"
#include "xfs_rtalloc.h"
#include "xfs_attr.h"
#include "xfs_ioctl.h"
#include "xfs_ioctl32.h"
#include "xfs_trace.h"
#include "xfs_sb.h"

#define  _NATIVE_IOC(cmd, type) \
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

		if (xfs_compat_growfs_data_copyin(&in, arg))
			return -EFAULT;
		error = mnt_want_write_file(filp);
		if (error)
			return error;
		error = xfs_growfs_data(ip->i_mount, &in);
		mnt_drop_write_file(filp);
		return error;
	}
	case XFS_IOC_FSGROWFSRT_32: {
		struct xfs_growfs_rt	in;

		if (xfs_compat_growfs_rt_copyin(&in, arg))
			return -EFAULT;
		error = mnt_want_write_file(filp);
		if (error)
			return error;
		error = xfs_growfs_rt(ip->i_mount, &in);
		mnt_drop_write_file(filp);
		return error;
	}
#endif
	/* long changes size, but xfs only copiese out 32 bits */
	case XFS_IOC_GETVERSION_32:
		cmd = _NATIVE_IOC(cmd, long);
		return xfs_file_ioctl(filp, cmd, p);
	case XFS_IOC_SWAPEXT_32: {
		struct xfs_swapext	  sxp;
		struct compat_xfs_swapext __user *sxu = arg;

		/* Bulk copy in up to the sx_stat field, then copy bstat */
		if (copy_from_user(&sxp, sxu,
				   offsetof(struct xfs_swapext, sx_stat)) ||
		    xfs_ioctl32_bstat_copyin(&sxp.sx_stat, &sxu->sx_stat))
			return -EFAULT;
		error = mnt_want_write_file(filp);
		if (error)
			return error;
		error = xfs_ioc_swapext(&sxp);
		mnt_drop_write_file(filp);
		return error;
	}
	case XFS_IOC_FSBULKSTAT_32:
	case XFS_IOC_FSBULKSTAT_SINGLE_32:
	case XFS_IOC_FSINUMBERS_32:
		return xfs_compat_ioc_fsbulkstat(filp, cmd, arg);
	case XFS_IOC_FD_TO_HANDLE_32:
	case XFS_IOC_PATH_TO_HANDLE_32:
	case XFS_IOC_PATH_TO_FSHANDLE_32: {
		struct xfs_fsop_handlereq	hreq;

		if (xfs_compat_handlereq_copyin(&hreq, arg))
			return -EFAULT;
		cmd = _NATIVE_IOC(cmd, struct xfs_fsop_handlereq);
		return xfs_find_handle(cmd, &hreq);
	}
	case XFS_IOC_OPEN_BY_HANDLE_32: {
		struct xfs_fsop_handlereq	hreq;

		if (xfs_compat_handlereq_copyin(&hreq, arg))
			return -EFAULT;
		return xfs_open_by_handle(filp, &hreq);
	}
	case XFS_IOC_READLINK_BY_HANDLE_32: {
		struct xfs_fsop_handlereq	hreq;

		if (xfs_compat_handlereq_copyin(&hreq, arg))
			return -EFAULT;
		return xfs_readlink_by_handle(filp, &hreq);
	}
	case XFS_IOC_ATTRLIST_BY_HANDLE_32:
		return xfs_compat_attrlist_by_handle(filp, arg);
	case XFS_IOC_ATTRMULTI_BY_HANDLE_32:
		return xfs_compat_attrmulti_by_handle(filp, arg);
	default:
		/* try the native version */
		return xfs_file_ioctl(filp, cmd, (unsigned long)arg);
	}