	if (error) {
		ASSERT(xfs_is_shutdown(mp));
		return error;
	}

	xfs_ilock(ip, XFS_ILOCK_EXCL);
	xfs_trans_ijoin(tp, ip, 0);

	/*
	 * Do not update the on-disk file size.  If we update the on-disk file
	 * size and then the system crashes before the contents of the file are
	 * flushed to disk then the files may be full of holes (ie NULL files
	 * bug).
	 */
	error = xfs_itruncate_extents_flags(&tp, ip, XFS_DATA_FORK,
				XFS_ISIZE(ip), XFS_BMAPI_NODISCARD);
	if (error)
		goto err_cancel;

	error = xfs_trans_commit(tp);
	if (error)
		goto out_unlock;

	xfs_inode_clear_eofblocks_tag(ip);
	goto out_unlock;

err_cancel:
	/*
	 * If we get an error at this point we simply don't
	 * bother truncating the file.
	 */
	xfs_trans_cancel(tp);
out_unlock:
	xfs_iunlock(ip, XFS_ILOCK_EXCL);
	return error;
}

int
xfs_alloc_file_space(
	struct xfs_inode	*ip,
	xfs_off_t		offset,
	xfs_off_t		len)
{
	xfs_mount_t		*mp = ip->i_mount;
	xfs_off_t		count;
	xfs_filblks_t		allocated_fsb;
	xfs_filblks_t		allocatesize_fsb;
	xfs_extlen_t		extsz, temp;
	xfs_fileoff_t		startoffset_fsb;
	xfs_fileoff_t		endoffset_fsb;
	int			nimaps;
	int			rt;
	xfs_trans_t		*tp;
	xfs_bmbt_irec_t		imaps[1], *imapp;
	int			error;

	trace_xfs_alloc_file_space(ip);

	if (xfs_is_shutdown(mp))
		return -EIO;

	error = xfs_qm_dqattach(ip);
	if (error)
		return error;

	if (len <= 0)
		return -EINVAL;

	rt = XFS_IS_REALTIME_INODE(ip);
	extsz = xfs_get_extsz_hint(ip);

	count = len;
	imapp = &imaps[0];
	nimaps = 1;
	startoffset_fsb	= XFS_B_TO_FSBT(mp, offset);
	endoffset_fsb = XFS_B_TO_FSB(mp, offset + count);
	allocatesize_fsb = endoffset_fsb - startoffset_fsb;

	/*
	 * Allocate file space until done or until there is an error
	 */
	while (allocatesize_fsb && !error) {
		xfs_fileoff_t	s, e;
		unsigned int	dblocks, rblocks, resblks;

		/*
		 * Determine space reservations for data/realtime.
		 */
		if (unlikely(extsz)) {
			s = startoffset_fsb;
			do_div(s, extsz);
			s *= extsz;
			e = startoffset_fsb + allocatesize_fsb;
			div_u64_rem(startoffset_fsb, extsz, &temp);
			if (temp)
				e += temp;
			div_u64_rem(e, extsz, &temp);
			if (temp)
				e += extsz - temp;
		} else {
			s = 0;
			e = allocatesize_fsb;
		}

		/*
		 * The transaction reservation is limited to a 32-bit block
		 * count, hence we need to limit the number of blocks we are
		 * trying to reserve to avoid an overflow. We can't allocate
		 * more than @nimaps extents, and an extent is limited on disk
		 * to MAXEXTLEN (21 bits), so use that to enforce the limit.
		 */
		resblks = min_t(xfs_fileoff_t, (e - s), (MAXEXTLEN * nimaps));
		if (unlikely(rt)) {
			dblocks = XFS_DIOSTRAT_SPACE_RES(mp, 0);
			rblocks = resblks;
		} else {
			dblocks = XFS_DIOSTRAT_SPACE_RES(mp, resblks);
			rblocks = 0;
		}

		/*
		 * Allocate and setup the transaction.
		 */
		error = xfs_trans_alloc_inode(ip, &M_RES(mp)->tr_write,
				dblocks, rblocks, false, &tp);
		if (error)
			break;

		error = xfs_iext_count_may_overflow(ip, XFS_DATA_FORK,
				XFS_IEXT_ADD_NOSPLIT_CNT);
		if (error)
			goto error;

		error = xfs_bmapi_write(tp, ip, startoffset_fsb,
				allocatesize_fsb, XFS_BMAPI_PREALLOC, 0, imapp,
				&nimaps);
		if (error)
			goto error;

		/*
		 * Complete the transaction
		 */
		error = xfs_trans_commit(tp);
		xfs_iunlock(ip, XFS_ILOCK_EXCL);
		if (error)
			break;

		allocated_fsb = imapp->br_blockcount;

		if (nimaps == 0) {
			error = -ENOSPC;
			break;
		}

		startoffset_fsb += allocated_fsb;
		allocatesize_fsb -= allocated_fsb;
	}

	return error;

error:
	xfs_trans_cancel(tp);
	xfs_iunlock(ip, XFS_ILOCK_EXCL);
	return error;
}

static int
xfs_unmap_extent(
	struct xfs_inode	*ip,
	xfs_fileoff_t		startoffset_fsb,
	xfs_filblks_t		len_fsb,
	int			*done)
{
	struct xfs_mount	*mp = ip->i_mount;
	struct xfs_trans	*tp;
	uint			resblks = XFS_DIOSTRAT_SPACE_RES(mp, 0);
	int			error;

	error = xfs_trans_alloc_inode(ip, &M_RES(mp)->tr_write, resblks, 0,
			false, &tp);
	if (error)
		return error;

	error = xfs_iext_count_may_overflow(ip, XFS_DATA_FORK,
			XFS_IEXT_PUNCH_HOLE_CNT);
	if (error)
		goto out_trans_cancel;

	error = xfs_bunmapi(tp, ip, startoffset_fsb, len_fsb, 0, 2, done);
	if (error)
		goto out_trans_cancel;

	error = xfs_trans_commit(tp);
out_unlock:
	xfs_iunlock(ip, XFS_ILOCK_EXCL);
	return error;

out_trans_cancel:
	xfs_trans_cancel(tp);
	goto out_unlock;
}

/* Caller must first wait for the completion of any pending DIOs if required. */
int
xfs_flush_unmap_range(
	struct xfs_inode	*ip,
	xfs_off_t		offset,
	xfs_off_t		len)
{
	struct xfs_mount	*mp = ip->i_mount;
	struct inode		*inode = VFS_I(ip);
	xfs_off_t		rounding, start, end;
	int			error;

	rounding = max_t(xfs_off_t, mp->m_sb.sb_blocksize, PAGE_SIZE);
	start = round_down(offset, rounding);
	end = round_up(offset + len, rounding) - 1;

	error = filemap_write_and_wait_range(inode->i_mapping, start, end);
	if (error)
		return error;
	truncate_pagecache_range(inode, start, end);
	return 0;
}

int
xfs_free_file_space(
	struct xfs_inode	*ip,
	xfs_off_t		offset,
	xfs_off_t		len)
{
	struct xfs_mount	*mp = ip->i_mount;
	xfs_fileoff_t		startoffset_fsb;
	xfs_fileoff_t		endoffset_fsb;
	int			done = 0, error;

	trace_xfs_free_file_space(ip);

	error = xfs_qm_dqattach(ip);
	if (error)
		return error;

	if (len <= 0)	/* if nothing being freed */
		return 0;

	startoffset_fsb = XFS_B_TO_FSB(mp, offset);
	endoffset_fsb = XFS_B_TO_FSBT(mp, offset + len);

	/* We can only free complete realtime extents. */
	if (XFS_IS_REALTIME_INODE(ip) && mp->m_sb.sb_rextsize > 1) {
		startoffset_fsb = roundup_64(startoffset_fsb,
					     mp->m_sb.sb_rextsize);
		endoffset_fsb = rounddown_64(endoffset_fsb,
					     mp->m_sb.sb_rextsize);
	}

	/*
	 * Need to zero the stuff we're not freeing, on disk.
	 */
	if (endoffset_fsb > startoffset_fsb) {
		while (!done) {
			error = xfs_unmap_extent(ip, startoffset_fsb,
					endoffset_fsb - startoffset_fsb, &done);
			if (error)
				return error;
		}
	}

	/*
	 * Now that we've unmap all full blocks we'll have to zero out any
	 * partial block at the beginning and/or end.  iomap_zero_range is smart
	 * enough to skip any holes, including those we just created, but we
	 * must take care not to zero beyond EOF and enlarge i_size.
	 */
	if (offset >= XFS_ISIZE(ip))
		return 0;
	if (offset + len > XFS_ISIZE(ip))
		len = XFS_ISIZE(ip) - offset;
	error = iomap_zero_range(VFS_I(ip), offset, len, NULL,
			&xfs_buffered_write_iomap_ops);
	if (error)
		return error;

	/*
	 * If we zeroed right up to EOF and EOF straddles a page boundary we
	 * must make sure that the post-EOF area is also zeroed because the
	 * page could be mmap'd and iomap_zero_range doesn't do that for us.
	 * Writeback of the eof page will do this, albeit clumsily.
	 */
	if (offset + len >= XFS_ISIZE(ip) && offset_in_page(offset + len) > 0) {
		error = filemap_write_and_wait_range(VFS_I(ip)->i_mapping,
				round_down(offset + len, PAGE_SIZE), LLONG_MAX);
	}

	return error;
}

static int
xfs_prepare_shift(
	struct xfs_inode	*ip,
	loff_t			offset)
{
	struct xfs_mount	*mp = ip->i_mount;
	int			error;

	/*
	 * Trim eofblocks to avoid shifting uninitialized post-eof preallocation
	 * into the accessible region of the file.
	 */
	if (xfs_can_free_eofblocks(ip, true)) {
		error = xfs_free_eofblocks(ip);
		if (error)
			return error;
	}

	/*
	 * Shift operations must stabilize the start block offset boundary along
	 * with the full range of the operation. If we don't, a COW writeback
	 * completion could race with an insert, front merge with the start
	 * extent (after split) during the shift and corrupt the file. Start
	 * with the block just prior to the start to stabilize the boundary.
	 */
	offset = round_down(offset, mp->m_sb.sb_blocksize);
	if (offset)
		offset -= mp->m_sb.sb_blocksize;

	/*
	 * Writeback and invalidate cache for the remainder of the file as we're
	 * about to shift down every extent from offset to EOF.
	 */
	error = xfs_flush_unmap_range(ip, offset, XFS_ISIZE(ip));
	if (error)
		return error;

	/*
	 * Clean out anything hanging around in the cow fork now that
	 * we've flushed all the dirty data out to disk to avoid having
	 * CoW extents at the wrong offsets.
	 */
	if (xfs_inode_has_cow_data(ip)) {
		error = xfs_reflink_cancel_cow_range(ip, offset, NULLFILEOFF,
				true);
		if (error)
			return error;
	}

	return 0;
}

/*
 * xfs_collapse_file_space()
 *	This routine frees disk space and shift extent for the given file.
 *	The first thing we do is to free data blocks in the specified range
 *	by calling xfs_free_file_space(). It would also sync dirty data
 *	and invalidate page cache over the region on which collapse range
 *	is working. And Shift extent records to the left to cover a hole.
 * RETURNS:
 *	0 on success
 *	errno on error
 *
 */
int
xfs_collapse_file_space(
	struct xfs_inode	*ip,
	xfs_off_t		offset,
	xfs_off_t		len)
{
	struct xfs_mount	*mp = ip->i_mount;
	struct xfs_trans	*tp;
	int			error;
	xfs_fileoff_t		next_fsb = XFS_B_TO_FSB(mp, offset + len);
	xfs_fileoff_t		shift_fsb = XFS_B_TO_FSB(mp, len);
	bool			done = false;

	ASSERT(xfs_isilocked(ip, XFS_IOLOCK_EXCL));
	ASSERT(xfs_isilocked(ip, XFS_MMAPLOCK_EXCL));

	trace_xfs_collapse_file_space(ip);

	error = xfs_free_file_space(ip, offset, len);
	if (error)
		return error;

	error = xfs_prepare_shift(ip, offset);
	if (error)
		return error;

	error = xfs_trans_alloc(mp, &M_RES(mp)->tr_write, 0, 0, 0, &tp);
	if (error)
		return error;

	xfs_ilock(ip, XFS_ILOCK_EXCL);
	xfs_trans_ijoin(tp, ip, 0);

	while (!done) {
		error = xfs_bmap_collapse_extents(tp, ip, &next_fsb, shift_fsb,
				&done);
		if (error)
			goto out_trans_cancel;
		if (done)
			break;

		/* finish any deferred frees and roll the transaction */
		error = xfs_defer_finish(&tp);
		if (error)
			goto out_trans_cancel;
	}

	error = xfs_trans_commit(tp);
	xfs_iunlock(ip, XFS_ILOCK_EXCL);
	return error;

out_trans_cancel:
	xfs_trans_cancel(tp);
	xfs_iunlock(ip, XFS_ILOCK_EXCL);
	return error;
}

/*
 * xfs_insert_file_space()
 *	This routine create hole space by shifting extents for the given file.
 *	The first thing we do is to sync dirty data and invalidate page cache
 *	over the region on which insert range is working. And split an extent
 *	to two extents at given offset by calling xfs_bmap_split_extent.
 *	And shift all extent records which are laying between [offset,
 *	last allocated extent] to the right to reserve hole range.
 * RETURNS:
 *	0 on success
 *	errno on error
 */
int
xfs_insert_file_space(
	struct xfs_inode	*ip,
	loff_t			offset,
	loff_t			len)
{
	struct xfs_mount	*mp = ip->i_mount;
	struct xfs_trans	*tp;
	int			error;
	xfs_fileoff_t		stop_fsb = XFS_B_TO_FSB(mp, offset);
	xfs_fileoff_t		next_fsb = NULLFSBLOCK;
	xfs_fileoff_t		shift_fsb = XFS_B_TO_FSB(mp, len);
	bool			done = false;

	ASSERT(xfs_isilocked(ip, XFS_IOLOCK_EXCL));
	ASSERT(xfs_isilocked(ip, XFS_MMAPLOCK_EXCL));

	trace_xfs_insert_file_space(ip);

	error = xfs_bmap_can_insert_extents(ip, stop_fsb, shift_fsb);
	if (error)
		return error;

	error = xfs_prepare_shift(ip, offset);
	if (error)
		return error;

	error = xfs_trans_alloc(mp, &M_RES(mp)->tr_write,
			XFS_DIOSTRAT_SPACE_RES(mp, 0), 0, 0, &tp);
	if (error)
		return error;

	xfs_ilock(ip, XFS_ILOCK_EXCL);
	xfs_trans_ijoin(tp, ip, 0);

	error = xfs_iext_count_may_overflow(ip, XFS_DATA_FORK,
			XFS_IEXT_PUNCH_HOLE_CNT);
	if (error)
		goto out_trans_cancel;

	/*
	 * The extent shifting code works on extent granularity. So, if stop_fsb
	 * is not the starting block of extent, we need to split the extent at
	 * stop_fsb.
	 */
	error = xfs_bmap_split_extent(tp, ip, stop_fsb);
	if (error)
		goto out_trans_cancel;

	do {
		error = xfs_defer_finish(&tp);
		if (error)
			goto out_trans_cancel;

		error = xfs_bmap_insert_extents(tp, ip, &next_fsb, shift_fsb,
				&done, stop_fsb);
		if (error)
			goto out_trans_cancel;
	} while (!done);

	error = xfs_trans_commit(tp);
	xfs_iunlock(ip, XFS_ILOCK_EXCL);
	return error;

out_trans_cancel:
	xfs_trans_cancel(tp);
	xfs_iunlock(ip, XFS_ILOCK_EXCL);
	return error;
}

/*
 * We need to check that the format of the data fork in the temporary inode is
 * valid for the target inode before doing the swap. This is not a problem with
 * attr1 because of the fixed fork offset, but attr2 has a dynamically sized
 * data fork depending on the space the attribute fork is taking so we can get
 * invalid formats on the target inode.
 *
 * E.g. target has space for 7 extents in extent format, temp inode only has
 * space for 6.  If we defragment down to 7 extents, then the tmp format is a
 * btree, but when swapped it needs to be in extent format. Hence we can't just
 * blindly swap data forks on attr2 filesystems.
 *
 * Note that we check the swap in both directions so that we don't end up with
 * a corrupt temporary inode, either.
 *
 * Note that fixing the way xfs_fsr sets up the attribute fork in the source
 * inode will prevent this situation from occurring, so all we do here is
 * reject and log the attempt. basically we are putting the responsibility on
 * userspace to get this right.
 */
static int
xfs_swap_extents_check_format(
	struct xfs_inode	*ip,	/* target inode */
	struct xfs_inode	*tip)	/* tmp inode */
{
	struct xfs_ifork	*ifp = &ip->i_df;
	struct xfs_ifork	*tifp = &tip->i_df;

	/* User/group/project quota ids must match if quotas are enforced. */
	if (XFS_IS_QUOTA_ON(ip->i_mount) &&
	    (!uid_eq(VFS_I(ip)->i_uid, VFS_I(tip)->i_uid) ||
	     !gid_eq(VFS_I(ip)->i_gid, VFS_I(tip)->i_gid) ||
	     ip->i_projid != tip->i_projid))
		return -EINVAL;

	/* Should never get a local format */
	if (ifp->if_format == XFS_DINODE_FMT_LOCAL ||
	    tifp->if_format == XFS_DINODE_FMT_LOCAL)
		return -EINVAL;

	/*
	 * if the target inode has less extents that then temporary inode then
	 * why did userspace call us?
	 */
	if (ifp->if_nextents < tifp->if_nextents)
		return -EINVAL;

	/*
	 * If we have to use the (expensive) rmap swap method, we can
	 * handle any number of extents and any format.
	 */
	if (xfs_has_rmapbt(ip->i_mount))
		return 0;

	/*
	 * if the target inode is in extent form and the temp inode is in btree
	 * form then we will end up with the target inode in the wrong format
	 * as we already know there are less extents in the temp inode.
	 */
	if (ifp->if_format == XFS_DINODE_FMT_EXTENTS &&
	    tifp->if_format == XFS_DINODE_FMT_BTREE)
		return -EINVAL;

	/* Check temp in extent form to max in target */
	if (tifp->if_format == XFS_DINODE_FMT_EXTENTS &&
	    tifp->if_nextents > XFS_IFORK_MAXEXT(ip, XFS_DATA_FORK))
		return -EINVAL;

	/* Check target in extent form to max in temp */
	if (ifp->if_format == XFS_DINODE_FMT_EXTENTS &&
	    ifp->if_nextents > XFS_IFORK_MAXEXT(tip, XFS_DATA_FORK))
		return -EINVAL;

	/*
	 * If we are in a btree format, check that the temp root block will fit
	 * in the target and that it has enough extents to be in btree format
	 * in the target.
	 *
	 * Note that we have to be careful to allow btree->extent conversions
	 * (a common defrag case) which will occur when the temp inode is in
	 * extent format...
	 */
	if (tifp->if_format == XFS_DINODE_FMT_BTREE) {
		if (XFS_IFORK_Q(ip) &&
		    XFS_BMAP_BMDR_SPACE(tifp->if_broot) > XFS_IFORK_BOFF(ip))
			return -EINVAL;
		if (tifp->if_nextents <= XFS_IFORK_MAXEXT(ip, XFS_DATA_FORK))
			return -EINVAL;
	}

	/* Reciprocal target->temp btree format checks */
	if (ifp->if_format == XFS_DINODE_FMT_BTREE) {
		if (XFS_IFORK_Q(tip) &&
		    XFS_BMAP_BMDR_SPACE(ip->i_df.if_broot) > XFS_IFORK_BOFF(tip))
			return -EINVAL;
		if (ifp->if_nextents <= XFS_IFORK_MAXEXT(tip, XFS_DATA_FORK))
			return -EINVAL;
	}

	return 0;
}

static int
xfs_swap_extent_flush(
	struct xfs_inode	*ip)
{
	int	error;

	error = filemap_write_and_wait(VFS_I(ip)->i_mapping);
	if (error)
		return error;
	truncate_pagecache_range(VFS_I(ip), 0, -1);

	/* Verify O_DIRECT for ftmp */
	if (VFS_I(ip)->i_mapping->nrpages)
		return -EINVAL;
	return 0;
}

/*
 * Move extents from one file to another, when rmap is enabled.
 */
STATIC int
xfs_swap_extent_rmap(
	struct xfs_trans		**tpp,
	struct xfs_inode		*ip,
	struct xfs_inode		*tip)
{
	struct xfs_trans		*tp = *tpp;
	struct xfs_bmbt_irec		irec;
	struct xfs_bmbt_irec		uirec;
	struct xfs_bmbt_irec		tirec;
	xfs_fileoff_t			offset_fsb;
	xfs_fileoff_t			end_fsb;
	xfs_filblks_t			count_fsb;
	int				error;
	xfs_filblks_t			ilen;
	xfs_filblks_t			rlen;
	int				nimaps;
	uint64_t			tip_flags2;

	/*
	 * If the source file has shared blocks, we must flag the donor
	 * file as having shared blocks so that we get the shared-block
	 * rmap functions when we go to fix up the rmaps.  The flags
	 * will be switch for reals later.
	 */
	tip_flags2 = tip->i_diflags2;
	if (ip->i_diflags2 & XFS_DIFLAG2_REFLINK)
		tip->i_diflags2 |= XFS_DIFLAG2_REFLINK;

	offset_fsb = 0;
	end_fsb = XFS_B_TO_FSB(ip->i_mount, i_size_read(VFS_I(ip)));
	count_fsb = (xfs_filblks_t)(end_fsb - offset_fsb);

	while (count_fsb) {
		/* Read extent from the donor file */
		nimaps = 1;
		error = xfs_bmapi_read(tip, offset_fsb, count_fsb, &tirec,
				&nimaps, 0);
		if (error)
			goto out;
		ASSERT(nimaps == 1);
		ASSERT(tirec.br_startblock != DELAYSTARTBLOCK);

		trace_xfs_swap_extent_rmap_remap(tip, &tirec);
		ilen = tirec.br_blockcount;

		/* Unmap the old blocks in the source file. */
		while (tirec.br_blockcount) {
			ASSERT(tp->t_firstblock == NULLFSBLOCK);
			trace_xfs_swap_extent_rmap_remap_piece(tip, &tirec);

			/* Read extent from the source file */
			nimaps = 1;
			error = xfs_bmapi_read(ip, tirec.br_startoff,
					tirec.br_blockcount, &irec,
					&nimaps, 0);
			if (error)
				goto out;
			ASSERT(nimaps == 1);
			ASSERT(tirec.br_startoff == irec.br_startoff);
			trace_xfs_swap_extent_rmap_remap_piece(ip, &irec);

			/* Trim the extent. */
			uirec = tirec;
			uirec.br_blockcount = rlen = min_t(xfs_filblks_t,
					tirec.br_blockcount,
					irec.br_blockcount);
			trace_xfs_swap_extent_rmap_remap_piece(tip, &uirec);

			if (xfs_bmap_is_real_extent(&uirec)) {
				error = xfs_iext_count_may_overflow(ip,
						XFS_DATA_FORK,
						XFS_IEXT_SWAP_RMAP_CNT);
				if (error)
					goto out;
			}

			if (xfs_bmap_is_real_extent(&irec)) {
				error = xfs_iext_count_may_overflow(tip,
						XFS_DATA_FORK,
						XFS_IEXT_SWAP_RMAP_CNT);
				if (error)
					goto out;
			}

			/* Remove the mapping from the donor file. */
			xfs_bmap_unmap_extent(tp, tip, &uirec);

			/* Remove the mapping from the source file. */
			xfs_bmap_unmap_extent(tp, ip, &irec);

			/* Map the donor file's blocks into the source file. */
			xfs_bmap_map_extent(tp, ip, &uirec);

			/* Map the source file's blocks into the donor file. */
			xfs_bmap_map_extent(tp, tip, &irec);

			error = xfs_defer_finish(tpp);
			tp = *tpp;
			if (error)
				goto out;

			tirec.br_startoff += rlen;
			if (tirec.br_startblock != HOLESTARTBLOCK &&
			    tirec.br_startblock != DELAYSTARTBLOCK)
				tirec.br_startblock += rlen;
			tirec.br_blockcount -= rlen;
		}

		/* Roll on... */
		count_fsb -= ilen;
		offset_fsb += ilen;
	}

	tip->i_diflags2 = tip_flags2;
	return 0;

out:
	trace_xfs_swap_extent_rmap_error(ip, error, _RET_IP_);
	tip->i_diflags2 = tip_flags2;
	return error;
}

/* Swap the extents of two files by swapping data forks. */
STATIC int
xfs_swap_extent_forks(
	struct xfs_trans	*tp,
	struct xfs_inode	*ip,
	struct xfs_inode	*tip,
	int			*src_log_flags,
	int			*target_log_flags)
{
	xfs_filblks_t		aforkblks = 0;
	xfs_filblks_t		taforkblks = 0;
	xfs_extnum_t		junk;
	uint64_t		tmp;
	int			error;

	/*
	 * Count the number of extended attribute blocks
	 */
	if (XFS_IFORK_Q(ip) && ip->i_afp->if_nextents > 0 &&
	    ip->i_afp->if_format != XFS_DINODE_FMT_LOCAL) {
		error = xfs_bmap_count_blocks(tp, ip, XFS_ATTR_FORK, &junk,
				&aforkblks);
		if (error)
			return error;
	}
	if (XFS_IFORK_Q(tip) && tip->i_afp->if_nextents > 0 &&
	    tip->i_afp->if_format != XFS_DINODE_FMT_LOCAL) {
		error = xfs_bmap_count_blocks(tp, tip, XFS_ATTR_FORK, &junk,
				&taforkblks);
		if (error)
			return error;
	}

	/*
	 * Btree format (v3) inodes have the inode number stamped in the bmbt
	 * block headers. We can't start changing the bmbt blocks until the
	 * inode owner change is logged so recovery does the right thing in the
	 * event of a crash. Set the owner change log flags now and leave the
	 * bmbt scan as the last step.
	 */
	if (xfs_has_v3inodes(ip->i_mount)) {
		if (ip->i_df.if_format == XFS_DINODE_FMT_BTREE)
			(*target_log_flags) |= XFS_ILOG_DOWNER;
		if (tip->i_df.if_format == XFS_DINODE_FMT_BTREE)
			(*src_log_flags) |= XFS_ILOG_DOWNER;
	}

	/*
	 * Swap the data forks of the inodes
	 */
	swap(ip->i_df, tip->i_df);

	/*
	 * Fix the on-disk inode values
	 */
	tmp = (uint64_t)ip->i_nblocks;
	ip->i_nblocks = tip->i_nblocks - taforkblks + aforkblks;
	tip->i_nblocks = tmp + taforkblks - aforkblks;

	/*
	 * The extents in the source inode could still contain speculative
	 * preallocation beyond EOF (e.g. the file is open but not modified
	 * while defrag is in progress). In that case, we need to copy over the
	 * number of delalloc blocks the data fork in the source inode is
	 * tracking beyond EOF so that when the fork is truncated away when the
	 * temporary inode is unlinked we don't underrun the i_delayed_blks
	 * counter on that inode.
	 */
	ASSERT(tip->i_delayed_blks == 0);
	tip->i_delayed_blks = ip->i_delayed_blks;
	ip->i_delayed_blks = 0;

	switch (ip->i_df.if_format) {
	case XFS_DINODE_FMT_EXTENTS:
		(*src_log_flags) |= XFS_ILOG_DEXT;
		break;
	case XFS_DINODE_FMT_BTREE:
		ASSERT(!xfs_has_v3inodes(ip->i_mount) ||
		       (*src_log_flags & XFS_ILOG_DOWNER));
		(*src_log_flags) |= XFS_ILOG_DBROOT;
		break;
	}

	switch (tip->i_df.if_format) {
	case XFS_DINODE_FMT_EXTENTS:
		(*target_log_flags) |= XFS_ILOG_DEXT;
		break;
	case XFS_DINODE_FMT_BTREE:
		(*target_log_flags) |= XFS_ILOG_DBROOT;
		ASSERT(!xfs_has_v3inodes(ip->i_mount) ||
		       (*target_log_flags & XFS_ILOG_DOWNER));
		break;
	}

	return 0;
}

/*
 * Fix up the owners of the bmbt blocks to refer to the current inode. The
 * change owner scan attempts to order all modified buffers in the current
 * transaction. In the event of ordered buffer failure, the offending buffer is
 * physically logged as a fallback and the scan returns -EAGAIN. We must roll
 * the transaction in this case to replenish the fallback log reservation and
 * restart the scan. This process repeats until the scan completes.
 */
static int
xfs_swap_change_owner(
	struct xfs_trans	**tpp,
	struct xfs_inode	*ip,
	struct xfs_inode	*tmpip)
{
	int			error;
	struct xfs_trans	*tp = *tpp;

	do {
		error = xfs_bmbt_change_owner(tp, ip, XFS_DATA_FORK, ip->i_ino,
					      NULL);
		/* success or fatal error */
		if (error != -EAGAIN)
			break;

		error = xfs_trans_roll(tpp);
		if (error)
			break;
		tp = *tpp;

		/*
		 * Redirty both inodes so they can relog and keep the log tail
		 * moving forward.
		 */
		xfs_trans_ijoin(tp, ip, 0);
		xfs_trans_ijoin(tp, tmpip, 0);
		xfs_trans_log_inode(tp, ip, XFS_ILOG_CORE);
		xfs_trans_log_inode(tp, tmpip, XFS_ILOG_CORE);
	} while (true);

	return error;
}

int
xfs_swap_extents(
	struct xfs_inode	*ip,	/* target inode */
	struct xfs_inode	*tip,	/* tmp inode */
	struct xfs_swapext	*sxp)
{
	struct xfs_mount	*mp = ip->i_mount;
	struct xfs_trans	*tp;
	struct xfs_bstat	*sbp = &sxp->sx_stat;
	int			src_log_flags, target_log_flags;
	int			error = 0;
	uint64_t		f;
	int			resblks = 0;
	unsigned int		flags = 0;

	/*
	 * Lock the inodes against other IO, page faults and truncate to
	 * begin with.  Then we can ensure the inodes are flushed and have no
	 * page cache safely. Once we have done this we can take the ilocks and
	 * do the rest of the checks.
	 */
	lock_two_nondirectories(VFS_I(ip), VFS_I(tip));
	filemap_invalidate_lock_two(VFS_I(ip)->i_mapping,
				    VFS_I(tip)->i_mapping);

	/* Verify that both files have the same format */
	if ((VFS_I(ip)->i_mode & S_IFMT) != (VFS_I(tip)->i_mode & S_IFMT)) {
		error = -EINVAL;
		goto out_unlock;
	}

	/* Verify both files are either real-time or non-realtime */
	if (XFS_IS_REALTIME_INODE(ip) != XFS_IS_REALTIME_INODE(tip)) {
		error = -EINVAL;
		goto out_unlock;
	}

	error = xfs_qm_dqattach(ip);
	if (error)
		goto out_unlock;

	error = xfs_qm_dqattach(tip);
	if (error)
		goto out_unlock;

	error = xfs_swap_extent_flush(ip);
	if (error)
		goto out_unlock;
	error = xfs_swap_extent_flush(tip);
	if (error)
		goto out_unlock;

	if (xfs_inode_has_cow_data(tip)) {
		error = xfs_reflink_cancel_cow_range(tip, 0, NULLFILEOFF, true);
		if (error)
			goto out_unlock;
	}

	/*
	 * Extent "swapping" with rmap requires a permanent reservation and
	 * a block reservation because it's really just a remap operation
	 * performed with log redo items!
	 */
	if (xfs_has_rmapbt(mp)) {
		int		w = XFS_DATA_FORK;
		uint32_t	ipnext = ip->i_df.if_nextents;
		uint32_t	tipnext	= tip->i_df.if_nextents;

		/*
		 * Conceptually this shouldn't affect the shape of either bmbt,
		 * but since we atomically move extents one by one, we reserve
		 * enough space to rebuild both trees.
		 */
		resblks = XFS_SWAP_RMAP_SPACE_RES(mp, ipnext, w);
		resblks +=  XFS_SWAP_RMAP_SPACE_RES(mp, tipnext, w);

		/*
		 * If either inode straddles a bmapbt block allocation boundary,
		 * the rmapbt algorithm triggers repeated allocs and frees as
		 * extents are remapped. This can exhaust the block reservation
		 * prematurely and cause shutdown. Return freed blocks to the
		 * transaction reservation to counter this behavior.
		 */
		flags |= XFS_TRANS_RES_FDBLKS;
	}
	error = xfs_trans_alloc(mp, &M_RES(mp)->tr_write, resblks, 0, flags,
				&tp);
	if (error)
		goto out_unlock;

	/*
	 * Lock and join the inodes to the tansaction so that transaction commit
	 * or cancel will unlock the inodes from this point onwards.
	 */
	xfs_lock_two_inodes(ip, XFS_ILOCK_EXCL, tip, XFS_ILOCK_EXCL);
	xfs_trans_ijoin(tp, ip, 0);
	xfs_trans_ijoin(tp, tip, 0);


	/* Verify all data are being swapped */
	if (sxp->sx_offset != 0 ||
	    sxp->sx_length != ip->i_disk_size ||
	    sxp->sx_length != tip->i_disk_size) {
		error = -EFAULT;
		goto out_trans_cancel;
	}

	trace_xfs_swap_extent_before(ip, 0);
	trace_xfs_swap_extent_before(tip, 1);

	/* check inode formats now that data is flushed */
	error = xfs_swap_extents_check_format(ip, tip);
	if (error) {
		xfs_notice(mp,
		    "%s: inode 0x%llx format is incompatible for exchanging.",
				__func__, ip->i_ino);
		goto out_trans_cancel;
	}

	/*
	 * Compare the current change & modify times with that
	 * passed in.  If they differ, we abort this swap.
	 * This is the mechanism used to ensure the calling
	 * process that the file was not changed out from
	 * under it.
	 */
	if ((sbp->bs_ctime.tv_sec != VFS_I(ip)->i_ctime.tv_sec) ||
	    (sbp->bs_ctime.tv_nsec != VFS_I(ip)->i_ctime.tv_nsec) ||
	    (sbp->bs_mtime.tv_sec != VFS_I(ip)->i_mtime.tv_sec) ||
	    (sbp->bs_mtime.tv_nsec != VFS_I(ip)->i_mtime.tv_nsec)) {
		error = -EBUSY;
		goto out_trans_cancel;
	}

	/*
	 * Note the trickiness in setting the log flags - we set the owner log
	 * flag on the opposite inode (i.e. the inode we are setting the new
	 * owner to be) because once we swap the forks and log that, log
	 * recovery is going to see the fork as owned by the swapped inode,
	 * not the pre-swapped inodes.
	 */
	src_log_flags = XFS_ILOG_CORE;
	target_log_flags = XFS_ILOG_CORE;

	if (xfs_has_rmapbt(mp))
		error = xfs_swap_extent_rmap(&tp, ip, tip);
	else
		error = xfs_swap_extent_forks(tp, ip, tip, &src_log_flags,
				&target_log_flags);
	if (error)
		goto out_trans_cancel;

	/* Do we have to swap reflink flags? */
	if ((ip->i_diflags2 & XFS_DIFLAG2_REFLINK) ^
	    (tip->i_diflags2 & XFS_DIFLAG2_REFLINK)) {
		f = ip->i_diflags2 & XFS_DIFLAG2_REFLINK;
		ip->i_diflags2 &= ~XFS_DIFLAG2_REFLINK;
		ip->i_diflags2 |= tip->i_diflags2 & XFS_DIFLAG2_REFLINK;
		tip->i_diflags2 &= ~XFS_DIFLAG2_REFLINK;
		tip->i_diflags2 |= f & XFS_DIFLAG2_REFLINK;
	}

	/* Swap the cow forks. */
	if (xfs_has_reflink(mp)) {
		ASSERT(!ip->i_cowfp ||
		       ip->i_cowfp->if_format == XFS_DINODE_FMT_EXTENTS);
		ASSERT(!tip->i_cowfp ||
		       tip->i_cowfp->if_format == XFS_DINODE_FMT_EXTENTS);

		swap(ip->i_cowfp, tip->i_cowfp);

		if (ip->i_cowfp && ip->i_cowfp->if_bytes)
			xfs_inode_set_cowblocks_tag(ip);
		else
			xfs_inode_clear_cowblocks_tag(ip);
		if (tip->i_cowfp && tip->i_cowfp->if_bytes)
			xfs_inode_set_cowblocks_tag(tip);
		else
			xfs_inode_clear_cowblocks_tag(tip);
	}

	xfs_trans_log_inode(tp, ip,  src_log_flags);
	xfs_trans_log_inode(tp, tip, target_log_flags);

	/*
	 * The extent forks have been swapped, but crc=1,rmapbt=0 filesystems
	 * have inode number owner values in the bmbt blocks that still refer to
	 * the old inode. Scan each bmbt to fix up the owner values with the
	 * inode number of the current inode.
	 */
	if (src_log_flags & XFS_ILOG_DOWNER) {
		error = xfs_swap_change_owner(&tp, ip, tip);
		if (error)
			goto out_trans_cancel;
	}
	if (target_log_flags & XFS_ILOG_DOWNER) {
		error = xfs_swap_change_owner(&tp, tip, ip);
		if (error)
			goto out_trans_cancel;
	}

	/*
	 * If this is a synchronous mount, make sure that the
	 * transaction goes to disk before returning to the user.
	 */
	if (xfs_has_wsync(mp))
		xfs_trans_set_sync(tp);

	error = xfs_trans_commit(tp);

	trace_xfs_swap_extent_after(ip, 0);
	trace_xfs_swap_extent_after(tip, 1);

out_unlock_ilock:
	xfs_iunlock(ip, XFS_ILOCK_EXCL);
	xfs_iunlock(tip, XFS_ILOCK_EXCL);
out_unlock:
	filemap_invalidate_unlock_two(VFS_I(ip)->i_mapping,
				      VFS_I(tip)->i_mapping);
	unlock_two_nondirectories(VFS_I(ip), VFS_I(tip));
	return error;

out_trans_cancel:
	xfs_trans_cancel(tp);
	goto out_unlock_ilock;
}
		} else {
			dblocks = XFS_DIOSTRAT_SPACE_RES(mp, resblks);
			rblocks = 0;
		}

		/*
		 * Allocate and setup the transaction.
		 */
		error = xfs_trans_alloc_inode(ip, &M_RES(mp)->tr_write,
				dblocks, rblocks, false, &tp);
		if (error)
			break;

		error = xfs_iext_count_may_overflow(ip, XFS_DATA_FORK,
				XFS_IEXT_ADD_NOSPLIT_CNT);
		if (error)
			goto error;

		error = xfs_bmapi_write(tp, ip, startoffset_fsb,
				allocatesize_fsb, XFS_BMAPI_PREALLOC, 0, imapp,
				&nimaps);
		if (error)
			goto error;

		/*
		 * Complete the transaction
		 */
		error = xfs_trans_commit(tp);
		xfs_iunlock(ip, XFS_ILOCK_EXCL);
		if (error)
			break;

		allocated_fsb = imapp->br_blockcount;

		if (nimaps == 0) {
			error = -ENOSPC;
			break;
		}