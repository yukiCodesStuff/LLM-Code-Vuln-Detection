	return err;
}

static int nla_memdup_cookie(struct tc_action *a, struct nlattr **tb)
{
	a->act_cookie = kzalloc(sizeof(*a->act_cookie), GFP_KERNEL);
	if (!a->act_cookie)
		return -ENOMEM;

	a->act_cookie->data = nla_memdup(tb[TCA_ACT_COOKIE], GFP_KERNEL);
	if (!a->act_cookie->data) {
		kfree(a->act_cookie);
		return -ENOMEM;
	}
	a->act_cookie->len = nla_len(tb[TCA_ACT_COOKIE]);

	return 0;
}

struct tc_action *tcf_action_init_1(struct net *net, struct nlattr *nla,
				    struct nlattr *est, char *name, int ovr,
{
	struct tc_action *a;
	struct tc_action_ops *a_o;
	char act_name[IFNAMSIZ];
	struct nlattr *tb[TCA_ACT_MAX + 1];
	struct nlattr *kind;
	int err;
			goto err_out;
		if (nla_strlcpy(act_name, kind, IFNAMSIZ) >= IFNAMSIZ)
			goto err_out;
	} else {
		err = -EINVAL;
		if (strlcpy(act_name, name, IFNAMSIZ) >= IFNAMSIZ)
			goto err_out;
	if (err < 0)
		goto err_mod;

	if (tb[TCA_ACT_COOKIE]) {
		int cklen = nla_len(tb[TCA_ACT_COOKIE]);

		if (cklen > TC_COOKIE_MAX_SIZE) {
			err = -EINVAL;
			tcf_hash_release(a, bind);
			goto err_mod;
		}

		if (nla_memdup_cookie(a, tb) < 0) {
			err = -ENOMEM;
			tcf_hash_release(a, bind);
			goto err_mod;
		}
	}

	/* module count goes up only when brand new policy is created
	 * if it exists and is only bound to in a_o->init() then
err_mod:
	module_put(a_o->owner);
err_out:
	return ERR_PTR(err);
}

static void cleanup_a(struct list_head *actions, int ovr)