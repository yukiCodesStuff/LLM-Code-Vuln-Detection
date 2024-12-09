	if (fold) {
		f->id = fold->id;
		f->iif = fold->iif;
		f->res = fold->res;
		f->handle = fold->handle;

		f->tp = fold->tp;
		f->bkt = fold->bkt;
		new = false;
	}

	err = route4_set_parms(net, tp, base, f, handle, head, tb,
			       tca[TCA_RATE], new, flags, extack);
	if (err < 0)
		goto errout;

	h = from_hash(f->handle >> 16);
	fp = &f->bkt->ht[h];
	for (pfp = rtnl_dereference(*fp);
	     (f1 = rtnl_dereference(*fp)) != NULL;
	     fp = &f1->next)
		if (f->handle < f1->handle)
			break;

	tcf_block_netif_keep_dst(tp->chain->block);
	rcu_assign_pointer(f->next, f1);
	rcu_assign_pointer(*fp, f);

	if (fold) {
		th = to_hash(fold->handle);
		h = from_hash(fold->handle >> 16);
		b = rtnl_dereference(head->table[th]);
		if (b) {
			fp = &b->ht[h];
			for (pfp = rtnl_dereference(*fp); pfp;
			     fp = &pfp->next, pfp = rtnl_dereference(*fp)) {
				if (pfp == fold) {
					rcu_assign_pointer(*fp, fold->next);
					break;
				}
			}
		}
	}