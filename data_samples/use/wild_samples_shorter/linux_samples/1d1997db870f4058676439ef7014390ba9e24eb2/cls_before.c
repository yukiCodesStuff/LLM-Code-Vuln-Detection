	u8 mask, val;
	int err;

	if (!nfp_abm_u32_check_knode(alink->abm, knode, proto, extack)) {
		err = -EOPNOTSUPP;
		goto err_delete;
	}

	tos_off = proto == htons(ETH_P_IP) ? 16 : 20;

	/* Extract the DSCP Class Selector bits */
		if ((iter->val & cmask) == (val & cmask) &&
		    iter->band != knode->res->classid) {
			NL_SET_ERR_MSG_MOD(extack, "conflict with already offloaded filter");
			err = -EOPNOTSUPP;
			goto err_delete;
		}
	}

	if (!match) {
		match = kzalloc(sizeof(*match), GFP_KERNEL);
		if (!match) {
			err = -ENOMEM;
			goto err_delete;
		}

		list_add(&match->list, &alink->dscp_map);
	}
	match->handle = knode->handle;
	match->band = knode->res->classid;

err_delete:
	nfp_abm_u32_knode_delete(alink, knode);
	return err;
}

static int nfp_abm_setup_tc_block_cb(enum tc_setup_type type,
				     void *type_data, void *cb_priv)