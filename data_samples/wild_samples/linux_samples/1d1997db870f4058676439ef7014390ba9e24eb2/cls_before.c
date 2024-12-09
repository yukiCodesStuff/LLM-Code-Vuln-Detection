{
	struct nfp_abm_u32_match *match = NULL, *iter;
	unsigned int tos_off;
	u8 mask, val;
	int err;

	if (!nfp_abm_u32_check_knode(alink->abm, knode, proto, extack)) {
		err = -EOPNOTSUPP;
		goto err_delete;
	}
		    iter->band != knode->res->classid) {
			NL_SET_ERR_MSG_MOD(extack, "conflict with already offloaded filter");
			err = -EOPNOTSUPP;
			goto err_delete;
		}
		if (!match) {
			err = -ENOMEM;
			goto err_delete;
		}

		list_add(&match->list, &alink->dscp_map);
	}
	match->handle = knode->handle;
	match->band = knode->res->classid;
	match->mask = mask;
	match->val = val;

	err = nfp_abm_update_band_map(alink);
	if (err)
		goto err_delete;

	return 0;

err_delete:
	nfp_abm_u32_knode_delete(alink, knode);
	return err;
}

static int nfp_abm_setup_tc_block_cb(enum tc_setup_type type,
				     void *type_data, void *cb_priv)
{
	struct tc_cls_u32_offload *cls_u32 = type_data;
	struct nfp_repr *repr = cb_priv;
	struct nfp_abm_link *alink;

	alink = repr->app_priv;

	if (type != TC_SETUP_CLSU32) {
		NL_SET_ERR_MSG_MOD(cls_u32->common.extack,
				   "only offload of u32 classifier supported");
		return -EOPNOTSUPP;
	}