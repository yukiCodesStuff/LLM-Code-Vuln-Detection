
		memset(dst + 1, 0, sizeof(*xdst) - sizeof(*dst));
		xdst->flo.ops = &xfrm_bundle_fc_ops;
	} else
		xdst = ERR_PTR(-ENOBUFS);

	xfrm_policy_put_afinfo(afinfo);