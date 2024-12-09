	if (!esize) {
		/* Exponent is zero, result is 1 mod MOD, i.e., 1 or 0
		 * depending on if MOD equals 1.  */
		res->nlimbs = (msize == 1 && mod->d[0] == 1) ? 0 : 1;
		if (res->nlimbs) {
			if (mpi_resize(res, 1) < 0)
				goto enomem;
			rp = res->d;
			rp[0] = 1;
		}
		res->sign = 0;
		goto leave;
	}
