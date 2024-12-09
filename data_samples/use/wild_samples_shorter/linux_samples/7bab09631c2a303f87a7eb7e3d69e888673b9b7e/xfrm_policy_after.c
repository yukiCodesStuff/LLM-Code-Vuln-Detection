	struct xfrm_state *x_new[XFRM_MAX_DEPTH];
	struct xfrm_migrate *mp;

	/* Stage 0 - sanity checks */
	if ((err = xfrm_migrate_check(m, num_migrate)) < 0)
		goto out;

	if (dir >= XFRM_POLICY_MAX) {
		err = -EINVAL;
		goto out;
	}

	/* Stage 1 - find policy */
	if ((pol = xfrm_migrate_policy_find(sel, dir, type, net)) == NULL) {
		err = -ENOENT;
		goto out;