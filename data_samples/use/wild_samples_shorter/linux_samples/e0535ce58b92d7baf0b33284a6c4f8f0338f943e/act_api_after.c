	return err;
}

static struct tc_cookie *nla_memdup_cookie(struct nlattr **tb)
{
	struct tc_cookie *c = kzalloc(sizeof(*c), GFP_KERNEL);
	if (!c)
		return NULL;

	c->data = nla_memdup(tb[TCA_ACT_COOKIE], GFP_KERNEL);
	if (!c->data) {
		kfree(c);
		return NULL;
	}
	c->len = nla_len(tb[TCA_ACT_COOKIE]);

	return c;
}

struct tc_action *tcf_action_init_1(struct net *net, struct nlattr *nla,
				    struct nlattr *est, char *name, int ovr,
{
	struct tc_action *a;
	struct tc_action_ops *a_o;
	struct tc_cookie *cookie = NULL;
	char act_name[IFNAMSIZ];
	struct nlattr *tb[TCA_ACT_MAX + 1];
	struct nlattr *kind;
	int err;
			goto err_out;
		if (nla_strlcpy(act_name, kind, IFNAMSIZ) >= IFNAMSIZ)
			goto err_out;
		if (tb[TCA_ACT_COOKIE]) {
			int cklen = nla_len(tb[TCA_ACT_COOKIE]);

			if (cklen > TC_COOKIE_MAX_SIZE)
				goto err_out;

			cookie = nla_memdup_cookie(tb);
			if (!cookie) {
				err = -ENOMEM;
				goto err_out;
			}
		}
	} else {
		err = -EINVAL;
		if (strlcpy(act_name, name, IFNAMSIZ) >= IFNAMSIZ)
			goto err_out;
	if (err < 0)
		goto err_mod;

	if (name == NULL && tb[TCA_ACT_COOKIE]) {
		if (a->act_cookie) {
			kfree(a->act_cookie->data);
			kfree(a->act_cookie);
		}
		a->act_cookie = cookie;
	}

	/* module count goes up only when brand new policy is created
	 * if it exists and is only bound to in a_o->init() then
err_mod:
	module_put(a_o->owner);
err_out:
	if (cookie) {
		kfree(cookie->data);
		kfree(cookie);
	}
	return ERR_PTR(err);
}

static void cleanup_a(struct list_head *actions, int ovr)