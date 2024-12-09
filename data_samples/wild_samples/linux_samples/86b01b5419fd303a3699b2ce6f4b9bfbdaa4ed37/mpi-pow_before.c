	if (!esize) {
		/* Exponent is zero, result is 1 mod MOD, i.e., 1 or 0
		 * depending on if MOD equals 1.  */
		rp[0] = 1;
		res->nlimbs = (msize == 1 && mod->d[0] == 1) ? 0 : 1;
		res->sign = 0;
		goto leave;
	}

	/* Normalize MOD (i.e. make its most significant bit set) as required by
	 * mpn_divrem.  This will make the intermediate values in the calculation
	 * slightly larger, but the correct result is obtained after a final
	 * reduction using the original MOD value.  */
	mp = mp_marker = mpi_alloc_limb_space(msize);
	if (!mp)
		goto enomem;
	mod_shift_cnt = count_leading_zeros(mod->d[msize - 1]);
	if (mod_shift_cnt)
		mpihelp_lshift(mp, mod->d, msize, mod_shift_cnt);
	else
		MPN_COPY(mp, mod->d, msize);

	bsize = base->nlimbs;
	bsign = base->sign;
	if (bsize > msize) {	/* The base is larger than the module. Reduce it. */
		/* Allocate (BSIZE + 1) with space for remainder and quotient.
		 * (The quotient is (bsize - msize + 1) limbs.)  */
		bp = bp_marker = mpi_alloc_limb_space(bsize + 1);
		if (!bp)
			goto enomem;
		MPN_COPY(bp, base->d, bsize);
		/* We don't care about the quotient, store it above the remainder,
		 * at BP + MSIZE.  */
		mpihelp_divrem(bp + msize, 0, bp, bsize, mp, msize);
		bsize = msize;
		/* Canonicalize the base, since we are going to multiply with it
		 * quite a few times.  */
		MPN_NORMALIZE(bp, bsize);
	} else
		bp = base->d;

	if (!bsize) {
		res->nlimbs = 0;
		res->sign = 0;
		goto leave;
	}

	if (res->alloced < size) {
		/* We have to allocate more space for RES.  If any of the input
		 * parameters are identical to RES, defer deallocation of the old
		 * space.  */
		if (rp == ep || rp == mp || rp == bp) {
			rp = mpi_alloc_limb_space(size);
			if (!rp)
				goto enomem;
			assign_rp = 1;
		} else {
			if (mpi_resize(res, size) < 0)
				goto enomem;
			rp = res->d;
		}
	} else {		/* Make BASE, EXP and MOD not overlap with RES.  */
		if (rp == bp) {
			/* RES and BASE are identical.  Allocate temp. space for BASE.  */
			BUG_ON(bp_marker);
			bp = bp_marker = mpi_alloc_limb_space(bsize);
			if (!bp)
				goto enomem;
			MPN_COPY(bp, rp, bsize);
		}
		if (rp == ep) {
			/* RES and EXP are identical.  Allocate temp. space for EXP.  */
			ep = ep_marker = mpi_alloc_limb_space(esize);
			if (!ep)
				goto enomem;
			MPN_COPY(ep, rp, esize);
		}
		if (rp == mp) {
			/* RES and MOD are identical.  Allocate temporary space for MOD. */
			BUG_ON(mp_marker);
			mp = mp_marker = mpi_alloc_limb_space(msize);
			if (!mp)
				goto enomem;
			MPN_COPY(mp, rp, msize);
		}
	}