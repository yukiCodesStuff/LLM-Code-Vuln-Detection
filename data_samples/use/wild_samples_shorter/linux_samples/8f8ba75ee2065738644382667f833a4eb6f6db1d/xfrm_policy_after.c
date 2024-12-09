
		memset(dst + 1, 0, sizeof(*xdst) - sizeof(*dst));
		xdst->flo.ops = &xfrm_bundle_fc_ops;
		if (afinfo->init_dst)
			afinfo->init_dst(net, xdst);
	} else
		xdst = ERR_PTR(-ENOBUFS);

	xfrm_policy_put_afinfo(afinfo);